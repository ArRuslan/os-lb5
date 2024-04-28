#include <iostream>
#include <vector>
#include <cstdint>
#include <windows.h>
#include <psapi.h>


void task2_4() {
    SYSTEM_INFO sysInfo;
    MEMORYSTATUSEX memStatus;
    ZeroMemory(&memStatus, sizeof(memStatus));
    memStatus.dwLength = sizeof(memStatus);

    GetSystemInfo(&sysInfo);
    GlobalMemoryStatusEx(&memStatus);

    printf("Page size: %u\n", sysInfo.dwPageSize);
    printf("Min address: %p\n", sysInfo.lpMinimumApplicationAddress);
    printf("Max address: %p\n", sysInfo.lpMaximumApplicationAddress);
    printf("Alloc granularity: %u\n", sysInfo.dwAllocationGranularity);

    printf("Memory load: %u%%\n", memStatus.dwMemoryLoad);
    printf("Physical memory (available/total): %luMB/%luMB\n", memStatus.ullAvailPhys / 1024 / 1024, memStatus.ullTotalPhys / 1024 / 1024);
    printf("Pagefile (available/total): %luMB/%luMB\n", memStatus.ullAvailPageFile / 1024 / 1024, memStatus.ullTotalPageFile / 1024 / 1024);
    printf("Virtual memory (available/total): %luMB/%luMB\n", memStatus.ullTotalVirtual / 1024 / 1024, memStatus.ullTotalVirtual / 1024 / 1024);
}

void task3_5() {
    HMODULE hModule = GetModuleHandle(nullptr);
    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo));

    MEMORY_BASIC_INFORMATION memoryInfo;
    VirtualQueryEx(GetCurrentProcess(), moduleInfo.lpBaseOfDll, &memoryInfo, sizeof(memoryInfo));
    void* baseAddress = memoryInfo.AllocationBase;

    VirtualQueryEx(GetCurrentProcess(), baseAddress + moduleInfo.SizeOfImage, &memoryInfo, sizeof(memoryInfo));
    void* endAddress = memoryInfo.BaseAddress + memoryInfo.RegionSize;

    std::cout << "Base Address: " << baseAddress << std::endl;
    std::cout << "End Address: " << endAddress << std::endl;
}

int main() {
    task2_4();
    task3_5();

    std::string a;
    std::cin >> a;
    //system("pause");

    return 0;
}