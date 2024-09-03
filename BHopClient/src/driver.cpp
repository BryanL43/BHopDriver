#include "../headers/driver.h"

namespace driver {
	bool attachToProcess(HANDLE driverHandle, const DWORD pid) {
		Request r;
		r.pid = reinterpret_cast<HANDLE>(pid);

		return DeviceIoControl(driverHandle, codes::attach, &r, sizeof(r),
			&r, sizeof(r), nullptr, nullptr);
	}
}