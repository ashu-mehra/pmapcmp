#ifndef process_hpp
#define process_hpp

#include <inttypes.h>
#include <unistd.h>
#include <map>
#include <set>
#include <iostream>
#include "ppagelist.hpp"
#include "pathlist.hpp"

using namespace std;

class PPage;
class PPageList;
class PathList;

typedef struct ProcMapsLine {
public:
	string range;
	string perm;
	string offset;
	string dev;
	string inode;
	string pathname;

	friend ostream& operator<< (std::ostream& out, const ProcMapsLine &mapsLine);
} ProcMapsLine;

class Process {
private:
	uint32_t _pid;
	FILE * _pageMap;
	uint32_t _pageSize;

	map<uint64_t, const PPage *> _vtop;
	map<const PPage *, uint64_t> _ptov;
	set<const PPage*> _unique;

	PPageList *_ppageList;
	PathList *_pathList;
	
public:
	Process(uint32_t pid, PPageList *ppageList = NULL, PathList *pathList = NULL) : _pid(pid)
	{
		_pageSize = getpagesize();
		_ppageList = ppageList;
		_pathList = pathList;
		_pageMap = NULL;
	}

	Process(const Process &other)
	{
		_pid = other._pid;
		_pageSize = other._pageSize;
		_ppageList = other._ppageList;
		_pathList = other._pathList;
	}

	~Process()
	{
		if (_pageMap) {
			fclose(_pageMap);
		}
	}
	
	bool operator< (const Process &p) const
	{
		if (_pid < p._pid) {
			return true;
		} else {
			return false;
		}
	}

	void openPageMapFile() {
		char pageMapFile[512];
		sprintf(pageMapFile, "/proc/%u/pagemap", _pid);
		_pageMap = fopen(pageMapFile, "r");
	}

	uint32_t getpid() const { return _pid; }
	void parseMapsLine(string, ProcMapsLine &); 
	void processMapsFile(void);
	void addMapping(uint64_t, const PPage *);
	uint64_t getPFN(uint64_t);
	uint64_t readPageMapEntry(uint64_t);
	void displayVtoPMap() const;
	void displayPtoVMap() const;

};

class ProcessList
{
private:
	set<Process> _processSet;
public:
	void addProcess(int32_t pid, PPageList &pageList, PathList &pathList);
	set<Process> & getProcessSet() { return  _processSet; }
	void processMapFile(void);
};

#endif
