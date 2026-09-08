/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "timer.h"
#include "pisr.h"

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Periodic 1 ms tick interrupt callback that updates timer state.
 */
void increment_counter(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool counting;      /**< Flag indicating whether the timer is actively running */
static bool finished;      /**< One-shot flag set when timer reaches target timeout */
static uint32_t max_count; /**< Target timeout period in milliseconds */
static uint32_t counter;   /**< Elapsed millisecond counter */

/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Registers periodic 1 ms callback with the PISR timer driver.
 * @return True if periodic timer callback registered successfully, false otherwise.
 */
bool timer_INIT(void)
{
    /* Register 1 millisecond periodic interrupt service handler */
    if(!pisrRegister(increment_counter, PISR_MS_TO_TICKS(1)))
    {
        return false;
    }

    return true;
}

/**
 * @brief Starts a non-blocking countdown timer.
 * @param time_ms Timeout duration in milliseconds.
 */
void start_timer_ms(uint32_t time_ms)
{
    counting  = true;
    max_count = time_ms;
    counter   = 0;
    finished  = false;
}

/**
 * @brief Resets current elapsed time counter to zero without stopping the timer.
 */
void reset_timer(void)
{
    counter = 0;
}

/**
 * @brief Polls timer status and automatically clears finished flag upon read.
 * @return True if timer has expired, false otherwise.
 */
bool timer_finished(void)
{
    if(finished)
    {
        finished = false; /* Clear flag on read (one-shot query) */
        return true;
    }

    return false;
}

/**
 * @brief Checks if the timer is currently active.
 * @return True if timer is counting, false if stopped or expired.
 */
bool timer_counting(void)
{
    return counting;
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINES & LOCAL FUNCTIONS
 ******************************************************************************/

/**
 * @brief Periodic PISR tick callback executed every 1 millisecond.
 */
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