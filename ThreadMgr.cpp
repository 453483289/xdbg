#include <Windows.h>
#include "ThreadMgr.h"
#include <tlhelp32.h>

ThreadMgr::ThreadMgr()
{
}


ThreadMgr::~ThreadMgr()
{
}

bool ThreadMgr::addAllThreads(DWORD excluded)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(hSnapshot, &te)) {
			do {
				if (te.th32OwnerProcessID == GetCurrentProcessId()) {

					if (te.th32ThreadID == excluded)
						continue;
					addThread(te.th32ThreadID);
				}

				te.dwSize = sizeof(te);
			} while (Thread32Next(hSnapshot, &te));
		}

		CloseHandle(hSnapshot);
	}

	return true;
}

void ThreadMgr::clearThreads()
{
	std::map<DWORD, HANDLE>::iterator it;
	for (it = _threads.begin(); it != _threads.end(); it++) {
		CloseHandle(it->second);
	}

	_threads.clear();
}

HANDLE ThreadMgr::addThread(DWORD tid)
{
	HANDLE hThread = openThread(THREAD_ALL_ACCESS, FALSE, tid);
	_threads[tid] = hThread;
	return hThread;
}

bool ThreadMgr::delThread(DWORD tid)
{
	_threads.erase(tid);
	return true;
}

void ThreadMgr::suspendAll(DWORD excluded)
{
	std::map<DWORD, HANDLE>::iterator it;
	for (it = _threads.begin(); it != _threads.end(); it++) {
		if (it->first == excluded)
			continue;
		suspendThread(it->second);
	}
}

void ThreadMgr::resumeAll(DWORD excluded)
{
	std::map<DWORD, HANDLE>::iterator it;
	for (it = _threads.begin(); it != _threads.end(); it++) {
		if (it->first == excluded)
			continue;
		resumeThread(it->second);
	}
}

HANDLE ThreadMgr::threadIdToHandle(DWORD tid)
{
	std::map<DWORD, HANDLE>::iterator it;
	it = _threads.find(tid);
	if (it == _threads.end())
		return NULL;
	return it->second;
}

DWORD ThreadMgr::threadHandleToId(HANDLE handle)
{
	std::map<DWORD, HANDLE>::iterator it;
	for (it = _threads.begin(); it != _threads.end(); it++) {
		if (it->second == handle)
			return it->first;
	}

	return 0;
}
