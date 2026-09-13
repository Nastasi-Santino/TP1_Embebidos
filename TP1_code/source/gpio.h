/***************************************************************************//**
  @file     gpio.h
  @brief    Simple GPIO Pin services driver interface
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _GPIO_H_
#define _GPIO_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/**
 * @brief Hardware Port Indices.
 */
enum { PA, PB, PC, PD, PE };

/**
 * @brief Pack Port index and Pin number into a single composite Pin ID byte.
 * @details Example: PTB5  -> PORTNUM2PIN(PB, 5)  -> 0x25
 *                   PTC22 -> PORTNUM2PIN(PC, 22) -> 0x56
 */
#define PORTNUM2PIN(p,n)    (((p)<<5) + (n))

/** Extracts Port index from composite Pin ID byte */
#define PIN2PORT(p)         (((p)>>5) & 0x07)

/** Extracts Pin bit index (0-31) from composite Pin ID byte */
#define PIN2NUM(p)          ((p) & 0x1F)

/**
 * @brief Pin Operation Modes.
 */
#ifndef INPUT
#define INPUT               0  /**< Standard digital input mode */
#define OUTPUT              1  /**< Push-pull digital output mode */
#define INPUT_PULLUP        2  /**< Digital input mode with internal Pull-Up resistor */
#define INPUT_PULLDOWN      3  /**< Digital input mode with internal Pull-Down resistor */
#endif // INPUT

/**
 * @brief Digital Logic Levels.
 */
#ifndef LOW
#define LOW                 0  /**< Logic Low level (0V / GND) */
#define HIGH                1  /**< Logic High level (3.3V / VCC) */
#endif // LOW

/**
 * @brief GPIO Interrupt Modes.
 */
enum {
    GPIO_IRQ_MODE_DISABLE,      /**< Disable GPIO interrupt generation */
    GPIO_IRQ_MODE_RISING_EDGE,  /**< Trigger IRQ on rising edge */
    GPIO_IRQ_MODE_FALLING_EDGE, /**< Trigger IRQ on falling edge */
    GPIO_IRQ_MODE_BOTH_EDGES,   /**< Trigger IRQ on both rising and falling edges */

    GPIO_IRQ_CANT_MODES
};

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/** Type definition for composite pin identifier */
typedef uint8_t pin_t;

/** Type definition for pin interrupt callback function pointers */
typedef void (*pinIrqFun_t)(void);

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Configures the specified pin to behave either as an input or an output.
 * @param pin The target pin identifier (encoded using PORTNUM2PIN).
 * @param mode Target pin mode (INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN).
 */
void gpioMode (pin_t pin, uint8_t mode);

/**
 * @brief Configures pin interrupt trigger condition and registers callback handler.
 * @param pin The target pin identifier (encoded using PORTNUM2PIN).
 * @param irqMode Trigger condition (GPIO_IRQ_MODE_RISING_EDGE, FALLING_EDGE, BOTH_EDGES).
 * @param irqFun Callback function to execute when pin interrupt fires.
 * @return True if handler registration succeeded, false otherwise.
 */
bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun);

/**
 * @brief Writes a HIGH or LOW digital state to an output pin.
 * @param pin The target pin identifier (encoded using PORTNUM2PIN).
 * @param value Desired state (HIGH or LOW).
 */
void gpioWrite (pin_t pin, bool value);

/**
 * @brief Toggles state of a digital pin (HIGH <-> LOW).
 * @param pin The target pin identifier (encoded using PORTNUM2PIN).
 */
void gpioToggle (pin_t pin);

/**
 * @brief Reads current digital state of a pin.
 * @param pin The target pin identifier (encoded using PORTNUM2PIN).
 * @return Current digital state (HIGH or LOW).
 */
bool gpioRead (pin_t pin);

/**
 * @brief Configures digital glitch filter parameters for a pin.
 * @param pin The target pin identifier (encoded using PORTNUM2PIN).
 * @param clk Clock source selection.
 * @param count Filter sample count threshold (0 - 31).
 * @return True if parameters were valid and applied, false otherwise.
 */
bool gpioDigitalFilter(pin_t pin, bool clk, uint32_t count);

#endif /* _GPIO_H_ */