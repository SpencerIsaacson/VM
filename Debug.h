#include <windows.h>

__int64 freq, start, end, diff;
void StartTiming()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
}

void EndTiming()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&end);
	diff = ((end - start) * 1000000) / freq;
	unsigned int microseconds = (unsigned int)(diff & 0xffffffff);
	printf("It took %u microseconds", microseconds);	
}