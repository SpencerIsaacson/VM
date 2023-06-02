#include <windows.h>

__int64 freq, start, end, diff;
void start_timing()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
}

void end_timing()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	diff = ((end - start) * 1000000) / freq;
	unsigned int microseconds = (unsigned int)(diff & 0xffffffff);
	printf("It took %u microseconds", microseconds);	
}

#define profile(x, y) start_timing();x;end_timing();printf(" to %s\n", y);

#define profile_n(n, x, y) start_timing();for(int i = 0; i < n; i++)x;end_timing();printf(" to %s %d times\n", y, n);