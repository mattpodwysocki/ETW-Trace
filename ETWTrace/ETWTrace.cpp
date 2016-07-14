#include "stdafx.h"
#include <stdio.h>
#include <Windows.h>
#include <Evntcons.h>
#include <Evntrace.h>

#pragma once


struct ITraceConsumer {
	virtual void OnEventRecord(PEVENT_RECORD eventPointer) = 0;
};

ITraceConsumer* pConsumer = NULL;

class TraceSession
{

public:
	TraceSession(LPCTSTR szSessionName);
	~TraceSession();

public:
	bool Start();
	bool EnableProvider(const GUID& providerId, UCHAR level, ULONGLONG anyKeyword = 0, ULONGLONG allKeyword = 0);
	bool OpenTrace(ITraceConsumer *pConsumer);
	bool Process();
	bool CloseTrace();
	bool DisableProvider(const GUID& providerId);
	bool Stop();

	ULONG Status() const;
	LONGLONG PerfFreq() const;

private:
	LPTSTR _szSessionName;
	ULONG _status;
	EVENT_TRACE_PROPERTIES* _pSessionProperties;
	TRACEHANDLE hSession;
	EVENT_TRACE_LOGFILE _logFile;
	TRACEHANDLE _hTrace;

};

TraceSession::TraceSession(LPCTSTR szSessionName) : _szSessionName(_tcsdup(szSessionName))
{
}

TraceSession::~TraceSession(void)
{
	delete[]_szSessionName;
	delete _pSessionProperties;
}

bool TraceSession::Start()
{
	if (!_pSessionProperties) {
		const size_t buffSize = sizeof(EVENT_TRACE_PROPERTIES) + (_tcslen(_szSessionName) + 1) * sizeof(TCHAR);
		_pSessionProperties = reinterpret_cast<EVENT_TRACE_PROPERTIES *>(malloc(buffSize));
		ZeroMemory(_pSessionProperties, buffSize);
		_pSessionProperties->Wnode.BufferSize = buffSize;
		_pSessionProperties->Wnode.ClientContext = 1;
		_pSessionProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
		_pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	}

	// Create the trace session.
	_status = StartTrace(&hSession, _szSessionName, _pSessionProperties);

	return (_status == ERROR_SUCCESS);
}

bool TraceSession::EnableProvider(const GUID& providerId, UCHAR level, ULONGLONG anyKeyword, ULONGLONG allKeyword)
{
	_status = EnableTraceEx2(hSession, &providerId, EVENT_CONTROL_CODE_ENABLE_PROVIDER, level, anyKeyword, allKeyword, 0, NULL);
	return (_status == ERROR_SUCCESS);
}

VOID WINAPI EventRecordCallback(_In_ PEVENT_RECORD pEventRecord)
{
	//ITraceConsumer* pConsumer = reinterpret_cast<ITraceConsumer *>(pEventRecord->UserContext);
 	pConsumer->OnEventRecord(pEventRecord);
}

bool TraceSession::OpenTrace(ITraceConsumer *pConsumer)
{
	if (!pConsumer)
		return false;

	ZeroMemory(&_logFile, sizeof(EVENT_TRACE_LOGFILE));
	_logFile.LoggerName = _szSessionName;
	_logFile.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
	_logFile.EventRecordCallback = &EventRecordCallback;
	_logFile.Context = pConsumer;

	_hTrace = ::OpenTrace(&_logFile);
	return (_hTrace != 0);
}

bool TraceSession::Process()
{
	_status = ProcessTrace(&_hTrace, 1, NULL, NULL);
	return (_status == ERROR_SUCCESS);
}

bool TraceSession::CloseTrace()
{
	_status = ::CloseTrace(_hTrace);
	return (_status == ERROR_SUCCESS);
}

bool TraceSession::DisableProvider(const GUID& providerId)
{
	_status = EnableTraceEx2(hSession, &providerId, EVENT_CONTROL_CODE_DISABLE_PROVIDER, 0, 0, 0, 0, NULL);
	return (_status == ERROR_SUCCESS);
}

bool TraceSession::Stop()
{
	_status = ControlTrace(hSession, _szSessionName, _pSessionProperties, EVENT_TRACE_CONTROL_STOP);
	delete _pSessionProperties;
	_pSessionProperties = NULL;

	return (_status == ERROR_SUCCESS);
}

ULONG TraceSession::Status() const
{
	return _status;
}

LONGLONG TraceSession::PerfFreq() const
{
	return _logFile.LogfileHeader.PerfFreq.QuadPart;
}

struct NodeTraceConsumer : ITraceConsumer
{
	NodeTraceConsumer()
	{

	};

	virtual void OnEventRecord(PEVENT_RECORD eventPointer) {
		wprintf(L"Processor ID %d\n", eventPointer->EventHeader.ProcessId);
	};
};


int main()
{
	pConsumer = new NodeTraceConsumer();

	GUID nodeGuid;
	LPTSTR szSessionName = L"SampleTracer";

	ZeroMemory(&nodeGuid, sizeof(GUID));
	if (IIDFromString(L"{77754E9B-264B-4D8D-B981-E4135C1ECB0C}", &nodeGuid) != S_OK)
	{
		wprintf(L"Failed to get GUID from string");
		return 1;
	}

	TraceSession* pSession = new TraceSession(szSessionName);
	if (!pSession->Start()) {
		if (pSession->Status() == ERROR_ALREADY_EXISTS) {
			if (!pSession->Stop() || !pSession->Start()) {
				wprintf(L"Error in trace session %d", pSession->Status());
				goto cleanup;
			}
		}
	}

	if (!pSession->EnableProvider(nodeGuid, TRACE_LEVEL_VERBOSE))
	{
		wprintf(L"Error in enabling provider %d", pSession->Status());
		goto cleanup;
	}

	if (!pSession->OpenTrace(pConsumer))
	{
		wprintf(L"Error in opening trace %d", pSession->Status());
		goto cleanup;
	}

	if (!pSession->Process())
	{
		wprintf(L"Error in processing %d", pSession->Status());
		goto cleanup;
	}

cleanup:
	delete pSession;

	return 0;

}

