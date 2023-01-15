#pragma once
#include <iostream>
#include <Windows.h>

#include <md5.h>
#include <md5.cpp>

#pragma warning(disable:6386)
namespace Utils {
	class Hashes {
	private:
		static std::string GetRegValue(HKEY hKey, const std::wstring& valueName) {
			std::wstring wValue;
			WCHAR szBuffer[256];
			DWORD dwBufferSize = sizeof(szBuffer);

			LSTATUS status = RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, (LPBYTE)szBuffer, &dwBufferSize);
			if (status != ERROR_SUCCESS) exit(0);

			wValue = szBuffer;
			return std::string(wValue.begin(), wValue.end());
		}

		static std::string GetCompName() {
			WCHAR szBuffer[256];
			DWORD dwBufferSize = sizeof(szBuffer);

			LSTATUS status = GetComputerNameW(szBuffer, &dwBufferSize);
			if (status != TRUE) exit(0);

			std::wstring wCompName = szBuffer;
			return std::string(wCompName.begin(), wCompName.end());
		}
	public:
		static __forceinline std::string GetUnHash() {
			return md5(md5(getenv("USERNAME")));
		}

		static __forceinline std::string GetReHash() {
			HKEY hKey;

			// Get VimeWorld OSUUID
			LSTATUS status = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\VimeWorld", NULL, KEY_READ, &hKey);
			if (status != ERROR_SUCCESS) exit(0);

			std::string osuuid = GetRegValue(hKey, L"osuuid");

			// Get hardware guid
			status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\IDConfigDB\\Hardware Profiles\\0001", NULL, KEY_READ, &hKey);
			if (status != ERROR_SUCCESS) exit(0);

			std::string hardware = GetRegValue(hKey, L"HwProfileGuid");
			hardware = std::string(hardware.begin() + 1, hardware.end() - 1);

			// Get motherboard product
			status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\HardwareConfig\\Current", NULL, KEY_READ, &hKey);
			if (status != ERROR_SUCCESS) exit(0);

			std::string motherboard = GetRegValue(hKey, L"BaseBoardProduct");

			std::string compName = GetCompName();
			std::string bind = motherboard + hardware + compName + osuuid;
			return md5(md5(bind));
		}
	};
}