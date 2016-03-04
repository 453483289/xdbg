#pragma once

#include "Thread.h"
#include "Lock.h"

#include <list>

class XDbgProxy : public Thread
{
private:
	XDbgProxy(void);
	~XDbgProxy(void);

public:
	bool initialize(); // ��ʼ��
	static XDbgProxy& instance()
	{
		static XDbgProxy inst;
		return inst;
	}

	BOOL DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

	// TODO: Implement LOAD_DLL_DEBUG_EVENT, UNLOAD_DLL_DEBUG_EVENT By:
	//	HOOK NtMapViewOfSection & NtUnmapViewOfSection, CHECK DLL LIST WHEN THE SYSCALL RETURNED	
	// Ignore CREATE_PROCESS_DEBUG_EVENT & EXIT_PROCESS_DEBUG_EVENT & RIP_EVENT

protected:
	static LONG CALLBACK _VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo);
	static VOID CALLBACK _LdrDllNotification(ULONG NotificationReason, 
		union _LDR_DLL_NOTIFICATION_DATA* NotificationData, PVOID Context);
	VOID CALLBACK LdrDllNotification(ULONG NotificationReason, 
		union _LDR_DLL_NOTIFICATION_DATA* NotificationData, PVOID Context);

	LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo);
	bool createPipe();

	virtual long run();

	void postMsg(DEBUG_EVENT& event);

protected:
	HANDLE				_hPipe;
	bool				_initOK;
	EXCEPTION_RECORD	_lastException;
	int					_stopFlag;
	std::list<DEBUG_EVENT>	_events;
	Mutex				_mutex;
};
