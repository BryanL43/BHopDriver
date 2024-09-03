#include "../headers/driver.h"

// Our "real" entry point
NTSTATUS driverMain(PDRIVER_OBJECT driverObj, PUNICODE_STRING registryPath) {
	UNREFERENCED_PARAMETER(registryPath); // Turn off registry path. Not referenced.

	UNICODE_STRING driverName = {}; // char* but Window's C++ version
	RtlInitUnicodeString(&driverName, L"\\Device\\BHop_Driver");

	PDEVICE_OBJECT deviceObj = nullptr;
	NTSTATUS status = IoCreateDevice(driverObj, 0, &driverName, FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObj);

	if (status != STATUS_SUCCESS) {
		debugPrint("[-] Failed to create device driver!\n");
		return status;
	}

	debugPrint("[+] Device driver successfully created!\n");

	UNICODE_STRING symbolicLink = {};
	RtlInitUnicodeString(&symbolicLink, L"\\DosDevices\\BHop_Driver");

	status = IoCreateSymbolicLink(&symbolicLink, &driverName);
	if (status != STATUS_SUCCESS) {
		debugPrint("[-] Failed to establish symbolic link!\n");
		return status;
	}

	debugPrint("[+] Device symbolic link successfully established!\n");

	// Allow us to send small amounts of data between user/kernal mode
	SetFlag(deviceObj->Flags, DO_BUFFERED_IO);

	// Set the driver handlers to our functions with our logic
	driverObj->MajorFunction[IRP_MJ_CREATE] = driver::create;
	driverObj->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	driverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

	ClearFlag(deviceObj->Flags, DO_DEVICE_INITIALIZING);

	debugPrint("[+] Driver initialized successfully!\n");

	return status;
}

// KdMapper will call this "entry point" but params will be null.
NTSTATUS DriverEntry() {
	debugPrint("[+] Redirecting driver entry point!\n");

	UNICODE_STRING driverName = {}; // char* but Window's C++ version
	RtlInitUnicodeString(&driverName, L"\\Driver\\BHop_Driver");

	return IoCreateDriver(&driverName, &driverMain);
}