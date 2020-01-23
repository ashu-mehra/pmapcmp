#include <set>
#include <iomanip>
#include "process.hpp"
#include "ppagelist.hpp"
#include "pathlist.hpp"

using namespace std;

int main(int argc, char *argv[]) {
	PPageList ppageList;
	PathList pathList;
	ProcessList processList;
	int32_t pagesize = getpagesize();
	int32_t total = 0;
	int32_t sharedTotal = 0;

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			int32_t pid = atoi(argv[i]);
			processList.addProcess(pid, ppageList, pathList);
		}
	}
	processList.processMapFile();

	set<Path> & pathSet = pathList.getPathSet();
	set<Process> & processSet = processList.getProcessSet();

	for (const Path & path: pathSet) {
		Path *ppath = (Path *)&path;

		for (const Process & process: processSet) {
			ppath->createPrivatePageSet(&process);
		}
	}

	/* header */
	cout << std::right << std::setw(Path::maxLength) << "Pathname/Region" << "\t" << std::left << std::setw(10) << "Shared";
	for (const Process & process: processSet) {
		cout << std::left << std::setw(10) << process.getpid();
	}
	cout << endl;

	/* display path information in each row */
	for (const Path & path: pathSet) {
		Path *ppath = (Path *)&path;
		cout << std::right << std::setw(Path::maxLength) << path.getPathname() << "\t";	
		ppath->findSharedPages();
		int32_t sharedPages = path.sharedPageCount();

		cout << std::left << std::setw(10) << (sharedPages * pagesize / 1024);
		sharedTotal += sharedPages;

		for (const Process & process: processSet) {
			int32_t privatePages = ppath->countPrivatePages(&process);
			cout << std::left << std::setw(10) << (privatePages * pagesize / 1024);
		}
		cout << endl;
	}
	cout << endl;

	/* display total now */
	cout << std::right << std::setw(Path::maxLength) << "Total" << "\t" << std::left << std::setw(10) << (sharedTotal * pagesize / 1024);
	for (const Process & process: processSet) {
		total = 0;
		//cout << "Process: " << process.getpid() << endl;
		for (const Path & path: pathSet) {
			//cout << path.getPathname() << ": " << path.countPrivatePages(&process) << endl;
			total += path.countPrivatePages(&process);
		}
		cout << std::left << std::setw(10) << (total * pagesize / 1024);
	}
	cout << endl;
}
