#include <iomanip>
#include "pathlist.hpp"

/* Path methods */

size_t Path::maxLength = 0;

void 
Path::addPage(const PPage *ppage)
{
	_pageSet.insert(ppage);
}

void
Path::displayPages(void) const
{
	cout << pageCount() << " pages mapped to pathname " << _pathname << ": \n";
	for (const PPage *page : _pageSet) {
		cout << "\taddress: " << std::hex << page->getAddress() << std::dec << " owners: ";
		for (const Process *p: page->getOwners()) {
			cout << p->getpid() << " ";
		}
		cout << endl;
	}
}

int32_t
Path::pageCount(void) const
{
	return _pageSet.size();
}

void
Path::findSharedPages()
{
	for (const PPage *page : _pageSet) {
		if (page->isShared()) {
			_sharedPageSet.insert(page);	
		}
	}
}

void
Path::displaySharedPages(void) const
{
	set<const PPage *>::iterator itr;
	cout << sharedPageCount() << " shared pages mapped to pathname " << _pathname << ": \n";
	for (const PPage *page : _sharedPageSet) {
		cout << "\taddress: " << std::hex << page->getAddress() << std::dec << " owners: ";
		for (const Process *p: page->getOwners()) {
			cout << p->getpid() << " ";
		}
		cout << endl;
	}
}

int32_t
Path::sharedPageCount(void) const
{
	return _sharedPageSet.size();
}

void
Path::createPrivatePageSet(const Process *process)
{
	set<const PPage *> privatePages;

	for (const PPage *page : _pageSet) {
		if (!page->isShared() && page->ownedBy(process)) {
			privatePages.insert(page);
		}
	}
	if (!privatePages.empty()) {
		_privatePageMap.insert({process, privatePages});
	}
}

int32_t
Path::countPrivatePages(const Process *process) const
{
	map<const Process *, set<const PPage *> >::const_iterator itr = _privatePageMap.find(process);
	if (itr != _privatePageMap.end()) {
		return itr->second.size();
	} else {
		return 0;
	}
}

/* PathList methods */

const Path * PathList::getPath(const string pathname)
{
	auto search = _pathSet.find(pathname);
	if (search != _pathSet.end()) {
		return &(*search);
	} else {
		return NULL;
	}
}

const Path * PathList::addPath(const string pathname)
{
	return &*(_pathSet.insert(Path(pathname)).first);
}

void PathList::displayPaths()
{
	int32_t total = 0;
	int32_t sharedTotal = 0;
	int32_t pagesize = getpagesize();

	for (const Path &path : _pathSet) {
		((Path &)path).findSharedPages();
#if 0
		path.displayPages();
		path.displaySharedPages();
#endif
		int32_t sharedPages = path.sharedPageCount();

		if (sharedPages > 0) {
			cout << std::right << std::setw(Path::maxLength) << path.getPathname() << "\t" << std::left << (sharedPages * pagesize / 1024) << " KB" << endl;
			sharedTotal += sharedPages;
		}
#if 0
		total += path.pageCount();
		sharedTotal += path.sharedPageCount();
#endif
	}
	//cout << "Total number of pages mapped to pathnames: " << total << endl;
	cout << std::right << std::setw(Path::maxLength) << "Total" << "\t" << std::left << (sharedTotal * pagesize / 1024) << " KB" << endl;
}
