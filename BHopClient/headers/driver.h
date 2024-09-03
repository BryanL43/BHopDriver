#ifndef DRIVER_H
#define DRIVER_H

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

namespace driver {
	namespace codes { // Holds the ioctl codes
		// Used to setup the driver
		constexpr ULONG attach =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		// Read process memory
		constexpr ULONG read =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		// Write process memory
		constexpr ULONG write =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	// Share between user mode & kernal mode
	struct Request {
		HANDLE pid;
		PVOID target;
		PVOID buffer;

		SIZE_T size;
		SIZE_T returnSize;
	};

	bool attachToProcess(HANDLE driverHandle, const DWORD pid);

	template<class T>
	T readMemory(HANDLE driverHandle, const std::uintptr_t address);

	template<class T>
	void writeMemory(HANDLE driverHandle, const std::uintptr_t address, const T& value);
}

#include "../headers/driver.inl" // Inline file to link template functions

#endif