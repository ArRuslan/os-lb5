#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <map>
#include <windows.h>

void task1() {
    // Реалізувати алгоритми заміщення сторінок та порівняти ці алгоритми по кількості вивантаження - завантаження
    // TODO: lru, nru, fifo

    /*const std::vector pageReferences = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    const int numFrames = 3;

    std::cout << "Not Recently Used (NRU) Page Loads/Unloads: " << notRecentlyUsed(pageReferences, numFrames) << std::endl;
    std::cout << "Last Recently Used (LRU) Page Loads/Unloads: " << lastRecentlyUsed(pageReferences, numFrames) << std::endl;*/
}

void task7_get_free_blocks(std::map<void*, uint64_t>& blocks) {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    void* address = systemInfo.lpMinimumApplicationAddress;
    while (address < systemInfo.lpMaximumApplicationAddress) {
        MEMORY_BASIC_INFORMATION memoryInfo;
        if (VirtualQuery(address, &memoryInfo, sizeof(memoryInfo)) == sizeof(memoryInfo)) {
            if (memoryInfo.State == MEM_FREE) {
                if (!blocks.count(memoryInfo.BaseAddress) || blocks[memoryInfo.BaseAddress] != memoryInfo.RegionSize) {
                    blocks[memoryInfo.BaseAddress] = memoryInfo.RegionSize;
                    printf("Base Address: %p, Size: %lu\n", memoryInfo.BaseAddress, memoryInfo.RegionSize);
                }
            }
            address = memoryInfo.BaseAddress + memoryInfo.RegionSize;
        }
        else {
            break;
        }
    }
}

void task7() {
    auto blocks = std::map<void*, uint64_t>();
    printf("Blocks:\n");
    task7_get_free_blocks(blocks);

    const auto mem = VirtualAlloc(nullptr, 32 * 1024, MEM_COMMIT, PAGE_READWRITE);
    printf("\nBlocks (after allocation):\n");
    task7_get_free_blocks(blocks);

    VirtualFree(mem, 32 * 1024, MEM_DECOMMIT);
}

void* task9_alloc(const uint32_t size) {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    std::vector<std::tuple<uint64_t, void*>> regions;

    for (void* address = systemInfo.lpMinimumApplicationAddress; address < systemInfo.lpMaximumApplicationAddress;) {
        MEMORY_BASIC_INFORMATION memoryInfo;
        if (VirtualQuery(address, &memoryInfo, sizeof(memoryInfo))) {
            void* reserveAddr = memoryInfo.BaseAddress + (uint64_t)memoryInfo.BaseAddress % systemInfo.
                dwAllocationGranularity;
            void* endAddr = memoryInfo.BaseAddress + memoryInfo.RegionSize;

            if (memoryInfo.State == MEM_FREE && memoryInfo.RegionSize >= size && reserveAddr + size <= endAddr) {
                regions.emplace_back(memoryInfo.RegionSize, reserveAddr);
            }
        }

        address = memoryInfo.BaseAddress + memoryInfo.RegionSize;
    }

    std::sort(regions.begin(), regions.end(), [](auto const& t1, auto const& t2) {
        return std::get<0>(t1) < std::get<0>(t2);
    });

    for(auto& region : regions) {
        printf("Trying to allocate at %p...\n", std::get<1>(region));
        void* ptr = VirtualAlloc(std::get<1>(region), size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (ptr)
            return ptr;
    }

    return nullptr;
}

void task9() {
    auto blocks = std::map<void*, uint64_t>();
    printf("\nBlocks (task 9):\n");
    task7_get_free_blocks(blocks);

    void* allocated = task9_alloc(32 * 1024);
    printf("\nAllocated at %p\n", allocated);
    printf("Blocks (after allocation):\n");
    task7_get_free_blocks(blocks);

    VirtualFree(allocated, 32 * 1024, MEM_DECOMMIT);
}

int main() {
    task1();
    // task7();
    // task9();

    return 0;
}
