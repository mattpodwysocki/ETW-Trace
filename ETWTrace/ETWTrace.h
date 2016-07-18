#pragma once

struct NodeProvider : ITraceConsumer
{
	NodeProvider() { }

	void OnEventRecord(_In_ PEVENT_RECORD pEventRecord)
	{
		wprintf(L"%d", pEventRecord->EventHeader.TimeStamp);
	}

	bool ContinueProcessing()
	{
		return TRUE;
	}
};