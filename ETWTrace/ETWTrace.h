#pragma once

#include "TraceEventInfo.h"

struct NodeProvider : ITraceConsumer
{
	// Time of the first event of the trace
	uint64_t mTraceStartTime = 0;

	NodeProvider() { }

	virtual void OnEventRecord(_In_ PEVENT_RECORD pEventRecord);
	virtual bool ContinueProcessing() { return TRUE; }
};

void NodeProvider::OnEventRecord(_In_ PEVENT_RECORD pEventRecord)
{
	if (mTraceStartTime == 0)
	{
		mTraceStartTime = pEventRecord->EventHeader.TimeStamp.QuadPart;
	}

	TraceEventInfo eventInfo(pEventRecord);
	wprintf(L"ProcessID %d\n", pEventRecord->EventHeader.ProcessId);

	wchar_t szGuidW[40] = { 0 };
	StringFromGUID2(pEventRecord->EventHeader.ProviderId, szGuidW, 40);
	wprintf(L"Provider GUID: %s\n", szGuidW);

	StringFromGUID2(pEventRecord->EventHeader.ActivityId, szGuidW, 40);
	wprintf(L"Activity GUID: %s\n", szGuidW);
}