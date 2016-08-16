
#include "tanlibrary/include/TrueAudioNext.h"   //TAN
#include "public/include/core/Platform.h"       //AMF
#include "public/common/AMFFactory.h"           //AMF

HINSTANCE           g_hTANModule = NULL;

TAN_SDK_LINK BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved
);


BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved
)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_hTANModule = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        // terminate AMF
        AMF_RESULT res = g_AMFFactory.Terminate();
        if (res != AMF_OK)
        {
            wprintf(L"AMF Factory Failed to terminate");
        }
    }
    return TRUE;
}

/*

enum AMF_COMPUTE_QUEUE_TYPE
{
    AMF_COMPUTE_QUEUE_NORMAL = 0,
    AMF_COMPUTE_QUEUE_MEDIUM_PRIORITY = 1
    AMF_COMPUTE_QUEUE_RESERVED_CU = 2
};
struct AMFRealTimeQueue
{
    AMF_COMPUTE_QUEUE_TYPE  queueType;
    int                     CU_COUNT;   //for 
};

*/
