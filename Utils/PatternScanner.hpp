#pragma once
#include <Windows.h>

namespace Utils {
	class PatternScanner {
	public:
		static bool compareBytes(PBYTE data, PBYTE pattern, PCHAR patternMask) {
			for (; *patternMask; data++, pattern++, patternMask++)
				if (*patternMask == 'x' && *data != *pattern)
					return false;
			return true;
		}

		static PBYTE scan(byte* pattern, char* mask) {
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);

			uintptr_t ui64MinimumApplicationAddress = reinterpret_cast<uintptr_t>(sysInfo.lpMinimumApplicationAddress);
			UINT64 offset = 0;

			while ((ui64MinimumApplicationAddress + offset) < reinterpret_cast<uintptr_t>(sysInfo.lpMaximumApplicationAddress)) {
				MEMORY_BASIC_INFORMATION mbi;
				VirtualQuery(reinterpret_cast<LPCVOID>(ui64MinimumApplicationAddress + offset), &mbi, sizeof(MEMORY_BASIC_INFORMATION));

				if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
					for (UINT64 i = 0; i < mbi.RegionSize; i++) {
						PBYTE address = reinterpret_cast<PBYTE>(reinterpret_cast<uintptr_t>(mbi.BaseAddress) + i);
						if (PatternScanner::compareBytes(address, pattern, mask))
							return address;
					}
				}

				offset += mbi.RegionSize;
			}

			return nullptr;
		}
	};
}