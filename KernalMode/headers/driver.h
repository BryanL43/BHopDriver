#ifndef DRIVER_H
#define DRIVER_H

#include <ntifs.h>

extern "C" {
	// Ensure our IOCTL driver is compatible with kdmapper
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName,
		PDRIVER_INITIALIZE InitializationFunction);

	// Implements read & write process memory in driver
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress,
		PEPROCESS TargetProcess, PVOID TargetAddress,
		SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize);
}

void debugPrint(PCSTR text); // Signature declaration for driver namespace

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

	NTSTATUS create(PDEVICE_OBJECT deviceObj, PIRP irp);
	NTSTATUS close(PDEVICE_OBJECT deviceObj, PIRP irp);
	NTSTATUS device_control(PDEVICE_OBJECT deviceObj, PIRP irp);
}

#endif