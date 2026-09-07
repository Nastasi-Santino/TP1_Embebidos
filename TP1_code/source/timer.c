#include "timer.h"
#include "pisr.h"

void increment_counter(void);

static bool counting;
static bool finished;
static uint32_t max_count;
static uint32_t counter;

bool timer_INIT(void)
{
	if(!pisrRegister(increment_counter , PISR_MS_TO_TICKS(1)))
	{
		return false;
	}

	return true;
}

void start_timer_ms(uint32_t time_ms)
{
	counting  = true;
	max_count = time_ms;
	counter = 0;
	finished = false;
}

void reset_timer(void)
{
	counter = 0;
}

bool timer_finished(void)
{
	if(finished)
	{
		finished = false;
		return true;
	}

	return false;
}

bool timer_counting(void)
{
	return counting;
}


void increment_counter(void)
{
	if(counting)
	{
		if(counter++ >= max_count)
		{
			finished = true;
			counting = false;
		}
	}
}
