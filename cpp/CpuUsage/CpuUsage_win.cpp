#include <iostream>
#include <Windows.h>
#include <signal.h>

BOOL WINAPI CtrlHandler(DWORD type) {
	if (type == CTRL_C_EVENT) {
		std::cout << "~Exit~\n";
		exit(0);
	}
	return false;
}

ULONGLONG difference(FILETIME &prev_kernel, FILETIME &prev_user, FILETIME &cur_kernel, FILETIME &cur_user) {
	LARGE_INTEGER a1, a2;
	a1.LowPart = prev_kernel.dwLowDateTime;
	a1.HighPart = prev_kernel.dwHighDateTime;
	a2.LowPart = prev_user.dwLowDateTime;
	a2.HighPart = prev_user.dwHighDateTime;

	LARGE_INTEGER b1, b2;
	b1.LowPart = cur_kernel.dwLowDateTime;
	b1.HighPart = cur_kernel.dwHighDateTime;
	b2.LowPart = cur_user.dwLowDateTime;
	b2.HighPart = cur_user.dwHighDateTime;
	
	//a1 and b1 - contains kernel times
	//a2 and b2 - contains user times
	return (b1.QuadPart - a1.QuadPart) + (b2.QuadPart - a2.QuadPart);
}

int main() {
	unsigned long pid = 0;
	// structure to store timings.
	FILETIME times1, times2;
	// for previous times.
	FILETIME prev_procees_times_kernel, prev_system_times_kernel;
	FILETIME prev_procees_times_user, prev_system_times_user;
	// for latest times.
	FILETIME procees_times_kernel, system_times_kernel;
	FILETIME procees_times_user, system_times_user;
	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	// Enter pid.
	std::cout << "Enter pid: ";
	std::cin >> pid;

	// Get process handle.
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, false, pid);
	if (process != NULL) {
		// First time run. Store initial times.
		if (GetProcessTimes(process, &times1, &times2, &prev_procees_times_kernel, &prev_procees_times_user) == NULL) {
			std::cerr << "Something went wrong. Exiting...\n";
			return 1;
		}
		if (GetSystemTimes(0, &prev_system_times_kernel, &prev_system_times_user) == NULL) {
			std::cerr << "Something went wrong. Exiting...\n";
			return 1;
		}
		while (true) {
			// Second time run. Store latest times.
			if (GetProcessTimes(process, &times1, &times2, &procees_times_kernel, &procees_times_user) == NULL) {
				std::cerr << "Something went wrong. Exiting...\n";
				break;
			}
			if (GetSystemTimes(0, &system_times_kernel, &system_times_user) == NULL) {
				std::cerr << "Something went wrong. Exiting...\n";
				break;
			}
			// Get diffrence latest - previous times.
			ULONGLONG proc = difference(prev_procees_times_kernel, prev_procees_times_user,
				procees_times_kernel, procees_times_user);
			ULONGLONG system = difference(prev_system_times_kernel, prev_system_times_user,
				system_times_kernel, system_times_user);
			double usage = 0.0;
			// Calcualte percentage.
			if(system != 0) usage = 100.0 * (proc / (double)system);
			std::cout << usage << "%\n";
			
			// Assign latest times to previous times for the next round of calculation.
			prev_procees_times_kernel = procees_times_kernel;
			prev_procees_times_user = procees_times_user;
			prev_system_times_kernel = system_times_kernel;
			prev_system_times_user = system_times_user;
			Sleep(5000);
		}
	}
	else {
		std::cerr << "Invalid pid or this process doen't have permission to read process pid " << pid << " info.";
	}
	std::cout << "Exit\n";
	return 1;
}

