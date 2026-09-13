/***************************************************************************//**
  @file     pisr.h
  @brief    Periodic Interrupt (PISR) driver interface
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _PISR_H_
#define _PISR_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PISR_TICK_US        10U  /**< Base SysTick periodic interrupt resolution in microseconds */

#define PISR_CANT           8    /**< Maximum allowed number of registered periodic callbacks */

/**
 * @brief Converts time duration from milliseconds to PISR ticks.
 */
#define PISR_MS_TO_TICKS(ms) \
    (((ms) * 1000U) / PISR_TICK_US)

/**
 * @brief Converts time duration from microseconds to PISR ticks.
 */
#define PISR_US_TO_TICKS(us) \
    ((us) / PISR_TICK_US)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/**
 * @brief Function pointer type for periodic interrupt callbacks.
 */
typedef void (*pisr_callback_t) (void);

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Registers a periodic interrupt service callback routine.
 * @param fun Callback function to be executed periodically.
 * @param period Execution period expressed in ticks.
 * @return True if callback registration succeeded, false otherwise.
 */
bool pisrRegister (pisr_callback_t fun, unsigned int period);

#endif /* _PISR_H_ */