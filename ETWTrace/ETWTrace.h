#pragma once

#include <string>
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

	auto opCode = pEventRecord->EventHeader.EventDescriptor.Opcode;
	auto task = pEventRecord->EventHeader.EventDescriptor.Task;
	auto eventId = pEventRecord->EventHeader.EventDescriptor.Id;

	wprintf(L"OpCode: %d\n", pEventRecord->EventHeader.EventDescriptor.Opcode);
	wprintf(L"Task: %d\n", pEventRecord->EventHeader.EventDescriptor.Task);
	wprintf(L"Event ID: %d\n", pEventRecord->EventHeader.EventDescriptor.Id);

	if (opCode == 10 &&
		task == 1 &&
		eventId == 9)
	{
		wprintf(L"-- JSCRIPT Runtime Task --\n");
		wprintf(L"Script Context ID %d\n", eventInfo.GetPtr(L"ScriptContextID"));
		wprintf(L"Method Start Address %d\n", eventInfo.GetPtr(L"MethodStartAddress"));
		wprintf(L"Method Size %d\n", eventInfo.GetData<uint64_t>(L"MethodSize"));
		wprintf(L"Method ID %d\n", eventInfo.GetData<uint32_t>(L"MethodID"));
		wprintf(L"Method Flags %d\n", eventInfo.GetData<uint16_t>(L"MethodFlags"));
		wprintf(L"Method Address Range ID %d\n", eventInfo.GetData<uint16_t>(L"MethodAddressRangeID"));
		wprintf(L"Source ID %d\n", eventInfo.GetData<uint64_t>(L"SourceID"));
		wprintf(L"Line %d\n", eventInfo.GetData<uint32_t>(L"Line"));
		wprintf(L"Column %d\n", eventInfo.GetData<uint32_t>(L"Column"));

		uint32_t dataSize = eventInfo.GetDataSize(L"MethodName");
		std::wstring data(L"");
		data.resize(dataSize, '\0');
		eventInfo.GetData(L"MethodName", (byte*)data.data(), dataSize);
		wprintf(L"Method Name: %s\n", data);
	}

	if ((opCode == 16 || opCode == 17) &&
		task == 0 &&
		(eventId == 7 || eventId == 8))
	{
		if (pEventRecord->EventHeader.EventDescriptor.Id == 7) {
			wprintf(L"-- Node GC Start --\n");
		} 
		else
		{
			wprintf(L"-- Node GC Done --\n");
		}
		
		wprintf(L"GC Type %d\n", eventInfo.GetData<uint32_t>(L"gctype"));
		wprintf(L"GC Callback Flags %d\n", eventInfo.GetData<uint32_t>(L"gccallbackflags"));
	}

	if (opCode == 23 &&
		task == 0 &&
		eventId == 23)
	{
		wprintf(L"-- Node V8 Symbol Reset --\n");
	}

	wprintf(L"\n");
}