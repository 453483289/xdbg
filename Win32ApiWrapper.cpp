#include <Windows.h>
#include <WinNt.h>
#include "Win32ApiWrapper.h"
#include "CloneFunction.h"
#include <assert.h>

#define STATUS_END_OF_FILE               ((NTSTATUS)0xC0000011L)

NTSTATUS
(NTAPI
* NtReadFile)(IN HANDLE FileHandle,
IN HANDLE Event OPTIONAL,
IN PVOID ApcRoutine OPTIONAL,
IN PVOID ApcContext OPTIONAL,
OUT PVOID IoStatusBlock,
OUT PVOID Buffer,
IN ULONG Length,
IN PLARGE_INTEGER ByteOffset OPTIONAL,
IN PULONG Key OPTIONAL) = NULL;

NTSTATUS
(NTAPI
* NtWriteFile)(IN HANDLE FileHandle,
IN HANDLE Event OPTIONAL,
IN PVOID ApcRoutine OPTIONAL,
IN PVOID ApcContext OPTIONAL,
OUT PVOID IoStatusBlock,
IN PVOID Buffer,
IN ULONG Length,
IN PLARGE_INTEGER ByteOffset OPTIONAL,
IN PULONG Key OPTIONAL) = NULL;

NTSTATUS (NTAPI * NtSuspendThread)(IN HANDLE ThreadHandle, OUT PULONG PreviousSuspendCount OPTIONAL) = NULL;
NTSTATUS(NTAPI * NtResumeThread)(IN HANDLE ThreadHandle, OUT PULONG SuspendCount OPTIONAL) = NULL;

NTSTATUS
(NTAPI
*NtWaitForSingleObject)(IN HANDLE ObjectHandle,
IN BOOLEAN Alertable,
IN PLARGE_INTEGER TimeOut  OPTIONAL) = NULL;

CloneFuncDef nativeApiDefs[] = {
	{ "ntdll.dll", "NtReadFile", (void** )&NtReadFile },
	{ "ntdll.dll", "NtWriteFile", (void**)&NtWriteFile },
	{ "ntdll.dll", "NtSuspendThread", (void**)&NtSuspendThread },
	{ "ntdll.dll", "NtResumeThread", (void**)&NtResumeThread },
	{ "ntdll.dll", "NtWaitForSingleObject", (void**)&NtWaitForSingleObject },
};


BOOL InitWin32ApiWrapper()
{
	return CloneFunctions(nativeApiDefs, sizeof(nativeApiDefs) / sizeof(nativeApiDefs[0]));
}

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	} DUMMYUNIONNAME;

	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

BOOL WINAPI XDbgReadFile(IN HANDLE hFile, IN LPVOID lpBuffer, IN DWORD nNumberOfBytesToRead,
	OUT LPDWORD lpNumberOfBytesRead OPTIONAL, IN LPOVERLAPPED lpOverlapped OPTIONAL)
{
	assert(lpOverlapped == NULL);

	IO_STATUS_BLOCK Iosb;

	NTSTATUS Status = NtReadFile(hFile,
		NULL,
		NULL,
		NULL,
		&Iosb,
		lpBuffer,
		nNumberOfBytesToRead,
		NULL,
		NULL);

	/* Wait in case operation is pending */
	if (Status == STATUS_PENDING)
	{
		Status = NtWaitForSingleObject(hFile, FALSE, NULL);
		if (Status == 0) Status = Iosb.Status;
	}

	if (Status == STATUS_END_OF_FILE)
	{
		/*
		* lpNumberOfBytesRead must not be NULL here, in fact Win doesn't
		* check that case either and crashes (only after the operation
		* completed).
		*/
		*lpNumberOfBytesRead = 0;
		return TRUE;
	}

	if (Status == 0)
	{
		/*
		* lpNumberOfBytesRead must not be NULL here, in fact Win doesn't
		* check that case either and crashes (only after the operation
		* completed).
		*/
		*lpNumberOfBytesRead = Iosb.Information;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI XDbgWriteFile(IN HANDLE hFile, IN LPCVOID lpBuffer, IN DWORD nNumberOfBytesToWrite OPTIONAL,
	OUT LPDWORD lpNumberOfBytesWritten OPTIONAL, IN LPOVERLAPPED lpOverlapped OPTIONAL)
{
	assert(lpOverlapped == NULL);

	IO_STATUS_BLOCK Iosb;

	NTSTATUS Status = NtWriteFile(hFile,
		NULL,
		NULL,
		NULL,
		&Iosb,
		(PVOID)lpBuffer,
		nNumberOfBytesToWrite,
		NULL,
		NULL);

	/* Wait in case operation is pending */
	if (Status == STATUS_PENDING)
	{
		Status = NtWaitForSingleObject(hFile, FALSE, NULL);
		if (Status == 0) Status = Iosb.Status;
	}

	if (Status == 0)
	{
		/*
		* lpNumberOfBytesWritten must not be NULL here, in fact Win doesn't
		* check that case either and crashes (only after the operation
		* completed).
		*/
		*lpNumberOfBytesWritten = Iosb.Information;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI XDbgGetCurrentProcessId()
{
	PVOID* teb = (PVOID* )NtCurrentTeb();
	return * LPDWORD(teb + 8);
}

DWORD WINAPI XDbgGetCurrentThreadId()
{
	PVOID* teb = (PVOID*)NtCurrentTeb();
	return *LPDWORD(teb + 9);
}

DWORD WINAPI XDbgSuspendThread(IN HANDLE hThread)
{
	DWORD PreviousSuspendCount;
	NTSTATUS status = NtSuspendThread(hThread, &PreviousSuspendCount);
	if (status) {
		return -1;
	}

	return PreviousSuspendCount;
}

DWORD WINAPI XDbgResumeThread(IN HANDLE hThread)
{
	DWORD SuspendCount;
	NTSTATUS status = NtSuspendThread(hThread, &SuspendCount);
	if (status) {
		return -1;
	}

	return SuspendCount;
}

HANDLE WINAPI XDbgGetCurrentProcess()
{
	return (HANDLE )-1;
}
