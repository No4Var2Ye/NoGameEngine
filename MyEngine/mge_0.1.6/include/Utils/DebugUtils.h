
// ======================================================================
#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__
// ======================================================================

// ======================================================================
#include <windows.h>
#include <psapi.h>
#include <string>
// ======================================================================
#pragma comment(lib, "psapi.lib")
// ======================================================================

namespace DebugUtils {

    /**
     * 获取当前进程使用的物理内存 (工作集大小)
     * @return 内存占用 (MB)
     */
    inline size_t GetUsedMemoryMB() {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize / (1024 * 1024);
        }
        return 0;
    }

    /**
     * 获取系统总物理内存
     * @return 总内存 (MB)
     */
    inline size_t GetTotalMemoryMB() {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            return (size_t)(memInfo.ullTotalPhys / (1024 * 1024));
        }
        return 0;
    }

    /**
     * 获取内存使用率
     * @return 0.0 - 100.0 的百分比
     */
    inline float GetMemoryUsagePercent() {
        size_t used = GetUsedMemoryMB();
        size_t total = GetTotalMemoryMB();
        if (total > 0) {
            return (static_cast<float>(used) / total) * 100.0f;
        }
        return 0.0f;
    }
}

#endif // __DEBUG_UTILS_H__