typedef struct _ADLAdapterInfo {
    int iAdapterIndex;
    bool active;
    int busNumber;
    int deviceNumber;
    int functionNumber;
    unsigned long long totalMemory;
    unsigned long long memoryBandwidth;
    int numCUs;
} ADLAdapterInfo;

int ADLQueryAdapterInfo(ADLAdapterInfo *adapterInfo, int listLen);