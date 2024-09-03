#ifndef DRIVER_INL
#define DRIVER_INL

namespace driver {
	template<class T>
	T readMemory(HANDLE driverHandle, const std::uintptr_t address) {
		T temp = {};

		Request r;
		r.target = reinterpret_cast<PVOID>(address);
		r.buffer = &temp;
		r.size = sizeof(T);

		DeviceIoControl(driverHandle, codes::read, &r, sizeof(r),
			&r, sizeof(r), nullptr, nullptr);

		return temp;
	}

	template<class T>
	void writeMemory(HANDLE driverHandle, const std::uintptr_t address, const T& value) {
		Request r;
		r.target = reinterpret_cast<PVOID>(address);
		r.buffer = (PVOID)&value;
		r.size = sizeof(T);

		DeviceIoControl(driverHandle, codes::write, &r, sizeof(r),
			&r, sizeof(r), nullptr, nullptr);
	}
}

#endif