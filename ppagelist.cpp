#include "ppagelist.hpp"

const PPage * PPageList::getPage(uint64_t pageAddress)
{
	auto search = _ppageSet.find(pageAddress);
	if (search != _ppageSet.end()) {
		return &(*search);
	} else {
		return NULL;
	}
}

const PPage * PPageList::addPage(uint64_t pageAddress)
{
	return &*(_ppageSet.insert(PPage(pageAddress)).first);
}

