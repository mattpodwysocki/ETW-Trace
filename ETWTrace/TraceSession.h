#ifndef _tracession_h_
#define  _tracesesion_h_

#include <windows.h>
#include <evntprov.h> // must be after windows.h
#include <evntrace.h> // must be after windows.h
#include <evntcons.h> // must be after windows.h
#include <stdint.h>

#undef OpenTrace

struct ITraceConsumer {
	virtual void OnEventRecord(_In_ PEVENT_RECORD pEventRecord) = 0;
	virtual bool ContinueProcessing() = 0;
};

class TraceSession
{

public:
	TraceSession(const wchar_t* szSessionName, const wchar_t* szFileName);
	~TraceSession();

public:
	bool Start();
	bool EnableProvider(const GUID& providerId, UCHAR level, ULONGLONG anyKeyword = 0, ULONGLONG allKeyword = 0);
	bool OpenTrace(ITraceConsumer *pConsumer);
	bool Process();
	bool CloseTrace();
	bool DisableProvider(const GUID& providerId);
	bool Stop();

	bool AnythingLost(uint32_t &events, uint32_t &buffers);

	ULONG Status() const;
	LONGLONG PerfFreq() const;

private:
	LPTSTR _szSessionName, _szFileName;
	ULONG _status;
	EVENT_TRACE_PROPERTIES* _pSessionProperties;
	TRACEHANDLE _hSession;
	EVENT_TRACE_LOGFILEW _logFile;
	TRACEHANDLE _hTrace;
	uint32_t _eventsLost, _buffersLost;
	bool _started;
};

#endif