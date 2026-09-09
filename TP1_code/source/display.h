#ifndef DISPLAY_H_
#define DISPLAY_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DISPLAY_COUNT       4U    /**< Total number of physical 7-segment display digits */
#define REFRESH_RATE_HZ     100U  /**< Screen refresh frequency in Hertz */

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/**
 * @brief Indicator LED activation combination modes.
 */
enum
{
    ALL_LEDS_OFF,          /**< Turn off all auxiliary status LEDs */
    ONLY_FIRST_LED,        /**< Illuminate only the 1st status LED */
    ONLY_SECOND_LED,       /**< Illuminate only the 2nd status LED */
    ONLY_THIRD_LED,        /**< Illuminate only the 3rd status LED */
    FIRST_AND_SECOND_LED,  /**< Alternating multiplex mode for 1st and 2nd LEDs */
    SECOND_AND_THIRD_LED,  /**< Alternating multiplex mode for 2nd and 3rd LEDs */
    FIRST_AND_THIRD_LED,   /**< Alternating multiplex mode for 1st and 3rd LEDs */
    ALL_LEDS_ON            /**< Multiplex mode illuminating all three status LEDs */
};

/**
 * @brief Software PWM display brightness levels (10% to 100%).
 */
enum
{
    TEN_PERCENT_BRIGTHNESS = 1,  /**< 10% display duty cycle brightness */
    TWENTY_PERCENT_BRIGTHNESS,   /**< 20% display duty cycle brightness */
    THIRTY_PERCENT_BRIGTHNESS,   /**< 30% display duty cycle brightness */
    FORTY_PERCENT_BRIGTHNESS,    /**< 40% display duty cycle brightness */
    FIFTY_PERCENT_BRIGTHNESS,    /**< 50% display duty cycle brightness */
    SIXTY_PERCENT_BRIGTHNESS,    /**< 60% display duty cycle brightness */
    SEVENTY_PERCENT_BRIGTHNESS,  /**< 70% display duty cycle brightness */
    EIGHTY_PERCENT_BRIGTHNESS,   /**< 80% display duty cycle brightness */
    NINETY_PERCENT_BRIGTHNESS,   /**< 90% display duty cycle brightness */
    HUNDRED_PERCENT_BRIGTHNESS   /**< 100% full display brightness */
};

/**
 * @brief Display rendering modes.
 */
enum
{
    EDITING,  /**< Active input mode with blinking/selected digit editing support */
    COMPLETE  /**< Static or scrolling message display mode */
};

/*******************************************************************************
 * CHARACTER TO TABLE INDEX ALIASES
 ******************************************************************************/

/* Keypad / Encoder Action Mappings for selectionEntered */
#define KEY_BACKSPACE        10  /**< Action: Delete last digit */
#define KEY_CLEAR            11  /**< Action: Clear active buffer */
#define KEY_BRIGHTNESS       12  /**< Action: Open brightness adjustment */
#define KEY_CHANGE_PASS      13  /**< Action: Initiate password change */
#define KEY_CANCEL           14  /**< Action: Cancel and return to start */
#define KEY_ENTER            15  /**< Action: Confirm/Submit input */


#define b		   12  /**< Map symbol 'b' to lookup table index 12 */
#define C          13  /**< Map symbol 'C' to lookup table index 13 */
#define G          6   /**< Map symbol 'G' to lookup table index 6 */
#define o          17  /**< Map symbol 'o' to lookup table index 17 */
#define d          18  /**< Map symbol 'd' to lookup table index 18 */
#define X          11  /**< Map symbol 'X' / 'H' to lookup table index 11 */
#define GUION      16  /**< Map symbol '-' (Dash) to lookup table index 16 */
#define I          19  /**< Map symbol 'I' to lookup table index 19 */
#define l          19  /**< Map symbol 'l' to lookup table index 19 */
#define n          20  /**< Map symbol 'n' to lookup table index 20 */
#define F          21  /**< Map symbol 'F' to lookup table index 21 */
#define P          22  /**< Map symbol 'P' to lookup table index 22 */
#define S          5   /**< Map symbol 'S' / '5' to lookup table index 5 */
#define a          23  /**< Map symbol 'a' / 'A' to lookup table index 23 */
#define t          24  /**< Map symbol 't' to lookup table index 24 */
#define APOSTROFE  25  /**< Map symbol '\'' (Apostrophe) to lookup table index 25 */
#define E          26  /**< Map symbol 'E' to lookup table index 26 */

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes display refresh hardware timer and serial transmitter.
 * @return True if periodic timer and serial interface initialized successfully, false otherwise.
 */
bool display_INIT(void);

/**
 * @brief Renders payload buffer on 7-segment display with specified settings.
 * @param data Pointer to input array of character/numeric table indices.
 * @param data_length Total length of the input data array.
 * @param selection Value/symbol to show at active cursor index in EDITING mode.
 * @param mode Render mode (EDITING or COMPLETE).
 * @param private True to obfuscate text (show dashes), false for plain output.
 * @param row Target line offset for scrolling multi-line data.
 * @param status Status indicator LED display combination.
 */
void print(uint8_t * data, uint8_t data_length,
        uint8_t selection, uint8_t mode, bool private, uint8_t row, uint8_t status);

/**
 * @brief Configures display brightness via software PWM duty cycle.
 * @param brightness Brightness scale level (ranging from 1 to 10).
 */
void setBrightness(uint8_t brightness);

#endif /* DISPLAY_H_ */
