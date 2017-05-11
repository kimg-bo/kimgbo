#include <time.h>

namespace kimgbo
{
	typedef timespec SystemTime; 
	
	void sleep(int milliseconds);

	SystemTime getSystemTime();

	// Returns the delta time, in milliseconds
	double getTimeDelta(SystemTime start);
}