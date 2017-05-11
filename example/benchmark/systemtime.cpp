#include "systemtime.h"
#include <climits>

#if defined(_MSC_VER) && _MSC_VER < 1700
#include <intrin.h>
#define CompilerMemBar() _ReadWriteBarrier()
#else
#include <atomic>
#define CompilerMemBar() std::atomic_signal_fence(std::memory_order_seq_cst)
#endif

#include <unistd.h>

namespace kimgbo
{
void sleep(int milliseconds)
{
	::usleep(milliseconds * 1000);
}

SystemTime getSystemTime()
{
	timespec t;
	CompilerMemBar();
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &t) != 0) {
		t.tv_sec = (time_t)-1;
		t.tv_nsec = -1;
	}
	CompilerMemBar();
	
	return t;
}

double getTimeDelta(SystemTime start)
{
	timespec t;
	CompilerMemBar();
	if ((start.tv_sec == (time_t)-1 && start.tv_nsec == -1) || clock_gettime(CLOCK_MONOTONIC_RAW, &t) != 0) {
		return -1;
	}
	CompilerMemBar();

	return static_cast<double>(static_cast<long>(t.tv_sec) - static_cast<long>(start.tv_sec)) * 1000 + double(t.tv_nsec - start.tv_nsec) / 1000000;
}

}  // end namespace kimgbo