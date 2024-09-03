#include "../headers/driver.h"

void debugPrint(PCSTR text) {
#ifndef DEBUG
	UNREFERENCED_PARAMETER(text);
#endif // Debug

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, text));
}

NTSTATUS driver::create(PDEVICE_OBJECT deviceObj, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObj);
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return irp->IoStatus.Status;
}

NTSTATUS driver::close(PDEVICE_OBJECT deviceObj, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObj);
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return irp->IoStatus.Status;
}

NTSTATUS driver::device_control(PDEVICE_OBJECT deviceObj, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObj);

	debugPrint("[+] Device control called!\n");

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	// Determines which code is passed through (holds attach, read, or write)
	PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

	// Access the request object sent from user mode
	auto request = reinterpret_cast<driver::Request*>(irp->AssociatedIrp.SystemBuffer);
	if (stack_irp == nullptr || request == nullptr) {
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return status;
	}

	// target process we want to read/write memory to
	static PEPROCESS targetProcess = nullptr;
	const ULONG controlCode = stack_irp->Parameters.DeviceIoControl.IoControlCode;
	switch (controlCode) {
	case driver::codes::attach:
		status = PsLookupProcessByProcessId(request->pid, &targetProcess);
		break;
	case driver::codes::read:
		if (targetProcess != nullptr) {
			status = MmCopyVirtualMemory(targetProcess, request->target, PsGetCurrentProcess(),
				request->buffer, request->size, KernelMode, &request->returnSize);
		}
		break;
	case driver::codes::write:
		if (targetProcess != nullptr) {
			status = MmCopyVirtualMemory(PsGetCurrentProcess(), request->buffer, targetProcess,
				request->target, request->size, KernelMode, &request->returnSize);
		}
		break;
	default:
		break;
	}

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = sizeof(driver::Request);

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}