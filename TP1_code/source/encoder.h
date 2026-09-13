/***************************************************************************//**
  @file     encoder.h
  @brief    Quadrature encoder and push-button driver interface
 ******************************************************************************/

#ifndef ENCODER_H_
#define ENCODER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/**
 * @brief Encoder rotation directions.
 */
enum {
    IS_LEFT,  /**< Counter-clockwise rotation */
    IS_RIGHT  /**< Clockwise rotation */
};

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes encoder hardware pins, edge interrupts, and PISR polling timers.
 * @return True if setup succeeded, false otherwise.
 */
bool encoder_INIT(void);

/**
 * @brief Polls push-button state.
 * @return True if button is currently held down, false otherwise.
 */
bool buttonPressed(void);

/**
 * @brief Checks if encoder was rotated and clears flag upon read.
 * @return True if rotation occurred, false otherwise.
 */
bool encoderMoved(void);

/**
 * @brief Returns rotation direction of the last valid movement event.
 * @return IS_LEFT or IS_RIGHT.
 */
bool encoderDir(void);

#endif /* ENCODER_H_ */