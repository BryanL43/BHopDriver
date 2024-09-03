#include "../headers/driver.h"

#include "client_dll.hpp"
#include "offsets.hpp"
#include "buttons.hpp"

static DWORD getPID(const wchar_t* processName) {
	DWORD pid = 0;

	HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snapShot == INVALID_HANDLE_VALUE) {
		return pid;
	}

	PROCESSENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Process32FirstW(snapShot, &entry) == TRUE) {
		// Check if the 1st handle is the one we want
		if (_wcsicmp(processName, entry.szExeFile) == 0) {
			pid = entry.th32ProcessID;
		}
		else {
			while (Process32NextW(snapShot, &entry) == TRUE) {
				if (_wcsicmp(processName, entry.szExeFile) == 0) {
					pid = entry.th32ProcessID;
					break;
				}
			}
		}
	}

	CloseHandle(snapShot);

	return pid;
}

static std::uintptr_t getModuleBase(const DWORD pid, const wchar_t* moduleName) {
	std::uintptr_t moduleBase = 0;

	HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snapShot == INVALID_HANDLE_VALUE) {
		return pid;
	}

	MODULEENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Module32FirstW(snapShot, &entry) == TRUE) {
		if (wcsstr(moduleName, entry.szModule) != nullptr) {
			moduleBase = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
		}
		else {
			while (Module32NextW(snapShot, &entry) == TRUE) {
				if (wcsstr(moduleName, entry.szModule) != nullptr) {
					moduleBase = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
					break;
				}
			}
		}
	}

	CloseHandle(snapShot);

	return moduleBase;
}

int main() {
	const DWORD pid = getPID(L"cs2.exe");
	if (pid == 0) {
		std::cout << "failed to find cs2!\n";
		std::cin.get();
		return 1;
	}

	const HANDLE driver = CreateFile(L"\\\\.\\BHop_Driver", GENERIC_READ, 0, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (driver == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to create our driver handle!\n";
		std::cin.get();
		return 1;
	}

	if (driver::attachToProcess(driver, pid) == true) {
		std::cout << "Attachment successful!\n";

		if (const std::uintptr_t client = getModuleBase(pid, L"client.dll"); client != 0) {
			std::cout << "Client found!\n";

			while (true) {
				if (GetAsyncKeyState(VK_END)) {
					break;
				}

				const auto localPlayerPawn = driver::readMemory<std::uintptr_t>(driver, client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);

				if (localPlayerPawn == 0) {
					continue;
				}

				const auto flags = driver::readMemory<std::uint32_t>(driver, localPlayerPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_fFlags);

				const bool inAir = flags & (1 << 0);
				const bool spacePressed = GetAsyncKeyState(VK_SPACE);
				const auto forceJump = driver::readMemory<DWORD>(driver, client + cs2_dumper::buttons::jump);

				if (spacePressed && inAir) {
					Sleep(5);
					driver::writeMemory(driver, client + cs2_dumper::buttons::jump, 65537);
				}
				else if (spacePressed && !inAir) {
					driver::writeMemory(driver, client + cs2_dumper::buttons::jump, 256);
				}
				else if (!spacePressed && forceJump == 65537) {
					driver::writeMemory(driver, client + cs2_dumper::buttons::jump, 256);
				}
			}
		}
	}

	CloseHandle(driver); // Prevent handle from being leaked to driver
	std::cin.get();

	return 0;
}