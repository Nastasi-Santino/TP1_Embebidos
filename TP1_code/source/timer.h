#ifndef TIMER_H_
#define TIMER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes 1 millisecond periodic timer tick handler via PISR.
 * @return True if periodic timer callback was successfully registered, false otherwise.
 */
bool timer_INIT(void);

/**
 * @brief Starts a non-blocking countdown timer.
 * @param time_ms Target timeout period in milliseconds.
 */
void start_timer_ms(uint32_t time_ms);

/**
 * @brief Resets current elapsed time counter back to zero without stopping the timer.
 */
void reset_timer(void);

/**
 * @brief Checks if the timer is currently active.
 * @return True if countdown is actively running, false if stopped or expired.
 */
bool timer_counting(void);

/**
 * @brief Polls timer timeout status and automatically clears finished flag upon read.
 * @return True if timer duration has elapsed, false otherwise.
 */
bool timer_finished(void);

#endif /* TIMER_H_ */