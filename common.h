#pragma once
#include <string>

template<typename T>
void* CastProcAddr(T p)
{
	union u {
		T		var;
		void*	f;
	} u1;

	u1.var = p;
	return u1.f;
};

static inline std::string makePipeName(DWORD pid)
{
	char buf[256];
	sprintf_s(buf, "\\\\.\\pipe\\__xdbg__%u__", pid);
	return buf;
}

#define EVENT_MESSAGE_SIZE		sizeof(DebugEventPacket)
#define CONTINUE_MESSAGE_SIZE	sizeof(DebugAckPacket)

struct DebugEventPacket {
	DEBUG_EVENT		event;
	CONTEXT			ctx;
};

struct DebugAckPacket {
	DWORD	dwProcessId;
	DWORD	dwThreadId;
	DWORD	dwContinueStatus;
	CONTEXT	ctx;
	DWORD	ContextFlags;
};

void _MyTrace(LPCSTR fmt, ...);

#ifdef _DEBUG
#define MyTrace		_MyTrace
#else
#define MyTrace
#endif

#define SINGLE_STEP_FLAG				0x100
#define DBG_PRINTEXCEPTION_WIDE_C		(0x4001000AL)

#ifdef _M_X64
#define CTX_PC_REG(CTX)		(CTX)->Rip
#else
#define CTX_PC_REG(CTX)		(CTX)->Eip
#endif // #ifdef _M_X64

#define ATTACHED_EVENT	(RIP_EVENT + 1)
#define LAST_EVENT		ATTACHED_EVENT
