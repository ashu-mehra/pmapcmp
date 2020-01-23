#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include "process.hpp"

using namespace std;

#define PAGE_SWAPPED_BIT (1ul << 62)
#define PAGE_PRESENT_BIT (1ul << 63)
#define PFN_MASK (0x7FFFFFFFFFFFFF) /* bits 0-54 */

void
Process::parseMapsLine(string line, ProcMapsLine &mapsLine)
{
	size_t length = line.length();
	size_t nextField = 0;
	size_t delim = 0;
	delim = line.find(' ');
	if (delim != string::npos) {
		mapsLine.range = line.substr(nextField, delim);
	}
	nextField = delim + 1;
	delim = line.find(' ', nextField);
	if (delim != string::npos) {
		mapsLine.perm = line.substr(nextField, delim - nextField);
	}
	nextField = delim + 1;
	delim = line.find(' ', nextField);
	if (delim != string::npos) {
		mapsLine.offset = line.substr(nextField, delim - nextField);
	}
	nextField = delim + 1;
	delim = line.find(' ', nextField);
	if (delim != string::npos) {
		mapsLine.dev = line.substr(nextField, delim - nextField);
	}
	nextField = delim + 1;
	delim = line.find(' ', nextField);
	if (delim != string::npos) {
		mapsLine.inode = line.substr(nextField, delim - nextField);
	}
	nextField = delim + 1;
	if (nextField < length) {
		delim = line.find(' ', nextField);
		while ((delim < length) && (line[delim] == ' ')) {
			delim += 1;
		}
		mapsLine.pathname = line.substr(delim);
	} else {
		mapsLine.pathname = "[anonymous mapping]";
	}
}

void
Process::processMapsFile(void) {
	ostringstream fileName;
	fileName << "/proc/" << _pid << "/maps";
	ifstream maps(fileName.str());

	if (maps.is_open() && (NULL != _pageMap)) {
		string line;
		while (getline(maps, line)) {
			ProcMapsLine mapsLine;
			parseMapsLine(line, mapsLine);
			cout << mapsLine;

			size_t hyphen = mapsLine.range.find('-');
			string from = line.substr(0, hyphen);
			string to = line.substr(hyphen+1);
			uint64_t fromAddr = stoull(from, NULL, 16);
			uint64_t toAddr = stoull(to, NULL, 16);
			uint64_t next = fromAddr;
			uint64_t pfn = 0;

			Path *path = (Path *)_pathList->addPath(mapsLine.pathname + "[" + mapsLine.perm + "]");

			do {
				pfn = getPFN(next);
				if (pfn != 0) {
					PPage * ppage = (PPage *)_ppageList->addPage(pfn);
					ppage->addOwner(this);
					ppage->setPath(path);
					cout << "Virtual address: " << std::hex << next << " ";
					cout << " PFN: " << std::hex << pfn << std::dec << endl;
					addMapping(next, ppage);
					path->addPage(ppage);
				}
				next += _pageSize;
			} while (next != toAddr);
		}
	}

	displayVtoPMap();
	displayPtoVMap();
	//_pathList->displayPaths();
}

void
Process::addMapping(uint64_t virtualPage, const PPage *ppage)
{
	pair<map<uint64_t, const PPage *>::iterator, bool> rc = _vtop.insert({virtualPage, ppage});
	if (!rc.second) {
		assert(!rc.second);
	}
	_ptov.insert({ppage, virtualPage});
}

uint64_t
Process::getPFN(uint64_t virtualAddress)
{
	uint64_t pageIndex = virtualAddress / _pageSize;
	uint64_t pageMapValue = readPageMapEntry(sizeof(uint64_t) * pageIndex);
	if (PAGE_PRESENT_BIT & pageMapValue) {
		return pageMapValue & PFN_MASK;
	} else {
		return 0;
	}
}

uint64_t
Process::readPageMapEntry(uint64_t offset)
{
	uint64_t value = 0;
	unsigned char singleByte;
	//cout << "pagemap offset: " << std::hex << offset << " ";
	if ( -1 != fseek(_pageMap, offset, SEEK_SET)) {
		for (int i = 0; i < sizeof(uint64_t); i++) {
			fscanf(_pageMap, "%c", &singleByte);
			value = (((uint64_t)singleByte) << (i * 8)) | value;	
		}
		//cout << "value: " << std::hex << value << std::dec << endl;
	}
	return value;
}

void
Process::displayVtoPMap(void) const {
	cout << "Virtual -> Physical, Entries=" << _vtop.size() << "\n";
	for (pair<uint64_t, const PPage *> element : _vtop) {
		uint64_t vaddr = element.first;
		const PPage *ppage = element.second;
		cout << std::hex << vaddr << "->" << ppage->getAddress() << std::dec << endl;
	}
}

void
Process::displayPtoVMap(void) const {
	cout << "Physical -> Virtual\n";
	for (pair<const PPage *, uint64_t> element : _ptov) {
		const PPage *ppage = element.first;
		uint64_t vaddr = element.second;
		cout << std::hex << ppage->getAddress() << "->" << vaddr << std::dec << endl;
	}
}

ostream& operator<< (std::ostream& out, const ProcMapsLine &mapsLine)
{
	return out << mapsLine.range << " " << mapsLine.perm << " " << mapsLine.offset << " " << mapsLine.dev << " " << mapsLine.inode << " " << mapsLine.pathname << endl;
}

void ProcessList::addProcess(int32_t pid, PPageList &pageList, PathList &pathList) 
{
	Process *p = (Process *)&*(_processSet.insert(Process(pid, &pageList, &pathList)).first);
	p->openPageMapFile();
}

void ProcessList::processMapFile(void)
{
	set<Process>::iterator itr;
	for (itr = _processSet.begin(); itr != _processSet.end(); itr++) {
		Process *p = (Process *)&*itr;
		cout << "Processing maps file for pid " << p->getpid() << endl;
		p->processMapsFile();
	}
}

