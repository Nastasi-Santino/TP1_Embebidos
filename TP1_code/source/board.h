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

/* =========================================================================
 * SERIAL_OUT PIN DEFINITIONS
 * ========================================================================= */

#define PIN_SERIAL      PORTNUM2PIN(PB, 11) /**< Serial Data input pin (PB11) */
#define PIN_SCLK        PORTNUM2PIN(PB, 2)  /**< Shift Register Clock pin (PB2) */
#define PIN_RCLK        PORTNUM2PIN(PB, 3)  /**< Storage Register / Latch Clock pin (PB3) */
#define PIN_OE          PORTNUM2PIN(PB, 10) /**< Output Enable pin (Active Low) (PB10) */

/* =========================================================================
 * ENCODER PIN DEFINITIONS
 * ========================================================================= */

#define PIN_SW_ENCODER          PORTNUM2PIN(PC, 11) /**< Push-button switch input pin (PC11) */
#define PIN_A_ENCODER           PORTNUM2PIN(PB, 18) /**< Quadrature Channel A input pin (PB18) */
#define PIN_B_ENCODER           PORTNUM2PIN(PC, 10) /**< Quadrature Channel B input pin (PC10) */

/* =========================================================================
 * CARD_READER PIN DEFINITIONS
 * ========================================================================= */

#define PIN_CR_ENABLE   PORTNUM2PIN(PD, 1) /**< Card reader Enable/Card-Present line (PTD1) */
#define PIN_CR_CLOCK    PORTNUM2PIN(PD, 3) /**< Card reader Clock input line (PTD3) */
#define PIN_CR_DATA     PORTNUM2PIN(PD, 2) /**< Card reader Active-Low Data input line (PTD2) */

/* =========================================================================
 * TEST POINTS PIN DEFINITIONS
 * ========================================================================= */

#define	PIN_TEST_POINT_PISR PORTNUM2PIN(PB, 23)
#define PIN_TEST_POINT_GPIO	PORTNUM2PIN(PC, 9)

/*******************************************************************************
 ******************************************************************************/

#endif /* _BOARD_H_ */
