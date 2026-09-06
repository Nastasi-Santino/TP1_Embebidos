#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
#include <stdbool.h>

bool timer_INIT(void);

void start_timer_ms(uint32_t time_ms);
void reset_timer(void);

bool timer_counting(void);
bool timer_finished(void);

#endif /* TIMER_H_ */
