#include "stdafx.h"
#include "TraceSession.h"
#include "ETWTrace.h"

struct __declspec(uuid("{77754E9B-264B-4D8D-B981-E4135C1ECB0C}")) NODEJS_PROVIDER_GUID_HOLDER;
static const auto NODEJS_PROVIDER_GUID = __uuidof(NODEJS_PROVIDER_GUID_HOLDER);

int main()
{
	const LPTSTR szSessionName = L"SampleTracer";
	NodeProvider provider = NodeProvider();

	TraceSession* pSession = new TraceSession(szSessionName, NULL);
	if (!pSession->Start()) {
		if (pSession->Status() == ERROR_ALREADY_EXISTS) {
			if (!pSession->Stop() || !pSession->Start()) {
				wprintf(L"Error in trace session %d", pSession->Status());
				goto cleanup;
			}
		}
	}

	if (!pSession->EnableProvider(NODEJS_PROVIDER_GUID, TRACE_LEVEL_VERBOSE))
	{
		wprintf(L"Error in enabling provider %d", pSession->Status());
		goto cleanup;
	}

	if (!pSession->OpenTrace(&provider))
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

