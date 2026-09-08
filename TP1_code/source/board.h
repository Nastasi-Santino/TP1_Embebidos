/***************************************************************************//**
  @file     board.h
  @brief    Board management
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"  /**< General Purpose Input/Output driver interface */

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/* =========================================================================
 * ON-BOARD USER LED PIN DEFINITIONS
 * ========================================================================= */

#define PIN_LED_RED     PORTNUM2PIN(PB, 22) /**< Red LED GPIO pin allocation (PTB22) */
#define PIN_LED_GREEN   PORTNUM2PIN(PE, 26) /**< Green LED GPIO pin allocation (PTE26) */
#define PIN_LED_BLUE    PORTNUM2PIN(PB, 21) /**< Blue LED GPIO pin allocation (PTB21) */

#define LED_ACTIVE      LOW                 /**< Logic level required to illuminate LEDs (Active Low) */

/* =========================================================================
 * ON-BOARD USER SWITCH PIN DEFINITIONS
 * ========================================================================= */

#define PIN_SW2         PORTNUM2PIN(PC, 6)  /**< Push button Switch 2 pin allocation (PTC6) */
#define PIN_SW3         PORTNUM2PIN(PA, 4)  /**< Push button Switch 3 pin allocation (PTA4) */

#define SW_ACTIVE       LOW                 /**< Logic level detected when switch is pressed (Active Low) */
#define SW_INPUT_TYPE   INPUT_PULLUP        /**< Input resistor configuration mode (Internal Pull-Up) */

/*******************************************************************************
 ******************************************************************************/

#endif /* _BOARD_H_ */
