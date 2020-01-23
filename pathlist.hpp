#ifndef pathlist_hpp
#define pathlist_hpp

#include <set>
#include <iostream>
#include "ppagelist.hpp"

using namespace std;

class PPage;
class PPageList;
class Process;

class Path {
private:
	string _pathname;
	set<const PPage *> _pageSet;
	set<const PPage *> _sharedPageSet;
	map<const Process *, set<const PPage *> > _privatePageMap;
public:
	static size_t maxLength;

	Path(string name) : _pathname(name) 
	{
		if (_pathname.length() > maxLength) {
			maxLength = _pathname.length();
		}
	}

	string getPathname() const {
		return _pathname;
	}

	void addPage(const PPage *ppage);
	void displayPages(void) const;
	void displaySharedPages(void) const;
	int32_t pageCount(void) const;
	int32_t sharedPageCount(void) const;
	void findSharedPages(void);
	void createPrivatePageSet(const Process *p);
	int32_t countPrivatePages(const Process *p) const;

	bool operator< (const Path &p) const {
		if (_pathname.compare(p._pathname) < 0) {
			return true;
		} else {
			return false;
		}
	}

	Path(const Path &other)
	{
		_pathname = other._pathname;
		_pageSet = other._pageSet;
	}
#if 0
	operator string() {
		return _pathname;
	}
#endif
};

class PathList
{
private:
	set<Path> _pathSet;

public:
	PathList() {}

	~PathList() {
	}

	const Path * getPath(const string pathname);
	const Path * addPath(const string pathname);
	set<Path> & getPathSet() { return _pathSet; }
	void displayPaths(void);
};

#endif
