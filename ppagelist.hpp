#ifndef ppagelist_hpp
#define ppagelist_hpp

#include <set>
#include <iostream>
#include "process.hpp"

using namespace std;

class Process;
class Path;

class Page {
private:
	uint64_t _address;

public:
	Page(uint64_t addr) : _address(addr) {}

	uint64_t getAddress() const {
		return _address;
	}

	bool operator< (const Page &p) const {
		if (_address < p._address) {
			return true;
		} else {
			return false;
		}
	}
};

class PPage : public Page {
private:
	set<const Process *> _owners;
	const Path *_path;

public:
	PPage(uint64_t addr) : Page(addr) {}

	void addOwner(const Process *p) {
		_owners.insert(p);
	}

	void setPath(const Path *path) {
		_path = path;
	}

	bool isShared() const {
		return _owners.size() > 1;
	}

	set<const Process *> getOwners() const {
		return _owners;
	}

	bool ownedBy(const Process *p) const {
		return (_owners.find(p) != _owners.end());
	}
};

class PPageList
{
private:
	set<PPage> _ppageSet;
public:
	const PPage * getPage(uint64_t pageAddress);
	const PPage * addPage(uint64_t pageAddress);
};

#endif
