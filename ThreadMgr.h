#pragma once
#include <map>
#include <assert.h>
#include "Lock.h"
class ThreadMgr
{
public:
	ThreadMgr();
	~ThreadMgr();

	bool addAllThreads(DWORD excluded);
	void clearThreads();
	HANDLE addThread(DWORD tid);
	bool delThread(DWORD tid);
	void suspendAll(DWORD excluded);
	void resumeAll(DWORD excluded);
	HANDLE threadIdToHandle(DWORD tid);
	DWORD threadHandleToId(HANDLE handle);

	DWORD getFirstThread() const
	{
		if (_threads.size() == 0) {
			assert(false);
			return 0;
		}

		return _threads.begin()->first;
	}

protected:
	virtual HANDLE openThread(DWORD dwDesiredAccess, BOOL bInheritHandle,
		DWORD dwThreadId)
	{
		return ::OpenThread(dwDesiredAccess, bInheritHandle, dwThreadId);
	}

	virtual DWORD suspendThread(HANDLE hThead)
	{
		return ::SuspendThread(hThead);
	}

	virtual DWORD resumeThread(HANDLE hThead)
	{
		return ::ResumeThread(hThead);
	}

protected:
	std::map<DWORD, HANDLE>	_threads;
};

