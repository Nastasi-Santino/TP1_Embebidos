/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "display.h"
#include "pisr.h"         /* Periodic Interrupt Service Routine driver */
#include "serial_out.h"   /* Shift register / SPI serial driver for display output */

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define REFRESH_PERIOD_US   (100000U / REFRESH_RATE_HZ)         /**< Display refresh period in microseconds */
#define COLUMN_PERIOD_US    (REFRESH_PERIOD_US / DISPLAY_COUNT)  /**< Multiplexing time slot per display digit column */

#define COLUMN_PERIOD_TICKS PISR_US_TO_TICKS(COLUMN_PERIOD_US)  /**< Microsecond-to-timer-ticks conversion */

/*******************************************************************************
 * PRIVATE DATA TYPES AND ENUMERATIONS
 ******************************************************************************/

/**
 * @brief Structure holding output segment data and active column target.
 */
typedef struct
{
    uint8_t seg;     /**< 7-segment bitmask byte (segments a-g + decimal point) */
    uint8_t column;  /**< Active column/digit index (0-3) */
} display_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Timer callback handling column multiplexing and software PWM duty cycle.
 */
void refreshColumns(void);

/**
 * @brief Converts a numeric/character code to a 7-segment bitmask.
 * @param num Index representing a number or character.
 * @param decimalPoint True to turn on the decimal point LED, false otherwise.
 * @return 8-bit segment drive bitmask.
 */
static uint8_t numberToSegments(uint8_t num, bool decimalPoint);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint8_t current_column;  /**< Index of the display digit currently active */
static bool update_data;         /**< Synchronizing flag set by ISR to trigger screen rendering */
static uint8_t DC = 10;          /**< Duty Cycle value (0-10) for PWM brightness control */
static bool off_data = false;    /**< Flag enabling blanking period for PWM dimming */

/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Initializes periodic multiplex timer and serial hardware driver.
 * @return True if periodic timer and serial interface initialized successfully, false otherwise.
 */
bool display_INIT(void)
{
    /* Register periodic display refresh callback with PISR timer driver */
    if(!pisrRegister(refreshColumns, COLUMN_PERIOD_TICKS))
    {
        return false;
    }

    /* Initialize shift register serial output peripheral */
    if(!serial_out_INIT())
    {
        return false;
    }

    return true;
}

/**
 * @brief Sets display brightness level via software PWM duty cycle.
 * @param brightness Duty cycle scaling value (ranging from 0 to 10).
 */
void setBrightness(uint8_t brightness)
{
    DC = brightness;
}

/**
 * @brief Formats input data and outputs active column, segments, and LED status flags.
 * @param data Pointer to input data array (digits or characters).
 * @param data_length Total length of input data buffer.
 * @param selection Current editing digit index or cursor selection.
 * @param mode Display mode (EDITING or COMPLETE).
 * @param private True to obfuscate digits (e.g., password entry with dashes), false for plain text.
 * @param row Target display row offset for multi-line scrollable text.
 * @param status Indicator LED state flags.
 */
void print(uint8_t * data, uint8_t data_length,
        uint8_t selection, uint8_t mode, bool private, uint8_t row, uint8_t status)
{
    /* Skip rendering if timer ISR hasn't requested a column update */
    if(!update_data)
    {
        return;
    }

    display_t output;
    uint8_t selection_column = 0;
    uint8_t index = 0;
    uint8_t current_column_temp = current_column;
    output.column = current_column_temp & 0x03;

    /* Mode 1: EDITING (renders input characters and flashing active selection cursor) */
    if(mode == EDITING && !off_data)
    {
        if(data_length >= DISPLAY_COUNT - 1)
        {
            selection_column = DISPLAY_COUNT - 1;
            index = data_length - DISPLAY_COUNT + 1 + current_column_temp;
        } else
        {
            selection_column = data_length;
            index = current_column_temp;
        }

        /* Active selection column displays decimal point or selected number */
        if(current_column_temp == selection_column)
        {
            output.seg = numberToSegments(selection, true);
        } else
        {
            if(index < data_length)
            {
                if(private)
                {
                    output.seg = numberToSegments(16, false); /* Dash sign for hidden entry */
                } else
                {
                    output.seg = numberToSegments(data[index], false);
                }
            } else
            {
                output.seg = 0x00; /* Blank display slot */
            }
        }
    } 
    /* Mode 2: COMPLETE (renders full static or scrolled messages) */
    else if(mode == COMPLETE && !off_data)
    {
        index = current_column_temp + row * DISPLAY_COUNT;
        if(index < data_length)
        {
            output.seg = numberToSegments(data[index], false);
        } else
        {
            output.seg = 0x00;
        }
    } 
    /* Blanking period active (PWM off phase) */
    else
    {
        output.seg = 0x00;
    }

    /* Process status indicator LED toggling/multiplexing states */
    static uint8_t current_led = 0;
    if(status < 4)
    {
        current_led = status;
    } else if(status == FIRST_AND_SECOND_LED)
    {
        current_led = (current_column_temp & 0x01) + 1;
    } else if(status == SECOND_AND_THIRD_LED)
    {
        current_led = (current_column_temp & 0x01) + 2;
    } else if(status == FIRST_AND_THIRD_LED)
    {
        current_led = ((current_led == 1) ? 3 : 1);
    } else if(status == ALL_LEDS_ON)
    {
        current_led = ((current_led == 3) ? 1 : current_led + 1);
    }

    /* Send byte payload to shift register output hardware */
    serial_out(output.seg, output.column, current_led);

    /* Acknowledge display frame update completion */
    if(current_column_temp == current_column)
    {
        update_data = false;
    }
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINES & LOCAL FUNCTIONS
 ******************************************************************************/

/**
 * @brief Periodic ISR callback handling software PWM duty cycle and column shifting.
 */
void refreshColumns(void)
{
    static uint8_t DC_counter;

    /* Disable display output when reaching the defined brightness threshold */
    if(DC_counter == DC)
    {
        update_data = true;
        off_data = true;
    }

    /* Complete full 10-step PWM period, then advance to next display digit */
    if(DC_counter == 10)
    {
        current_column = (current_column + 1) & 0x03;
        update_data = true;
        off_data = false;
        DC_counter = 0;
    } else
    {
        DC_counter++;
    }
}

/**
 * @brief Maps numeric indices to 7-segment display segment bitmasks.
 * @details Segment Mapping Bitmask:
 *          Bit 0 (0x01) -> Segment a (Top)
 *          Bit 1 (0x02) -> Segment b (Top-Right)
 *          Bit 2 (0x04) -> Segment c (Bottom-Right)
 *          Bit 3 (0x08) -> Segment d (Bottom)
 *          Bit 4 (0x10) -> Segment e (Bottom-Left)
 *          Bit 5 (0x20) -> Segment f (Top-Left)
 *          Bit 6 (0x40) -> Segment g (Middle)
 *          Bit 7 (0x80) -> Decimal Point (DP)
 *
 * @param num Index corresponding to the character to render (0 - 26).
 * @param decimalPoint True to illuminate the decimal point LED, false otherwise.
 * @return 8-bit segment drive bitmask.
 */
static uint8_t numberToSegments(uint8_t num, bool decimalPoint)
{
    static const uint8_t segments[] = {
        /* [Index] = Bitmask ,  Character rendered  (Active Segments) */
        /*  0 */     0x3F,   /* Digit '0'           (a, b, c, d, e, f)     */
        /*  1 */     0x06,   /* Digit '1'           (b, c)                 */
        /*  2 */     0x5B,   /* Digit '2'           (a, b, d, e, g)        */
        /*  3 */     0x4F,   /* Digit '3'           (a, b, c, d, g)        */
        /*  4 */     0x66,   /* Digit '4'           (b, c, f, g)           */
        /*  5 */     0x6D,   /* Digit '5'           (a, c, d, f, g)        */
        /*  6 */     0x7D,   /* Digit '6'           (a, c, d, e, f, g)     */
        /*  7 */     0x07,   /* Digit '7'           (a, b, c)              */
        /*  8 */     0x7F,   /* Digit '8'           (a, b, c, d, e, f, g)  */
        /*  9 */     0x6F,   /* Digit '9'           (a, b, c, d, f, g)     */

        /* 10 */     0x70,   /* Letter 'r' / Symbol (e, f, g)              */
        /* 11 */     0x76,   /* Letter 'H'          (b, c, e, f, g)        */
        /* 12 */     0x7C,   /* Letter 'b'          (c, d, e, f, g)        */
        /* 13 */     0x39,   /* Letter 'C'          (a, d, e, f)           */
        /* 14 */     0x50,   /* Letter 'r' (mini)   (e, g)                 */
        /* 15 */     0x1E,   /* Letter 'J'          (b, c, d, e)           */

        /* 16 */     0x40,   /* Symbol '-' (Dash)   (g)                    */

        /* 17 */     0x5C,   /* Letter 'o'          (c, d, e, g)           */
        /* 18 */     0x5E,   /* Letter 'd'          (b, c, d, e, g)        */
        /* 19 */     0x06,   /* Letter 'I'          (b, c)                 */
        /* 20 */     0x54,   /* Letter 'n'          (c, e, g)              */
        /* 21 */     0x71,   /* Letter 'F'          (a, e, f, g)           */
        /* 22 */     0x73,   /* Letter 'P'          (a, b, e, f, g)        */
        /* 23 */     0x77,   /* Letter 'A'          (a, b, c, e, f, g)     */
        /* 24 */     0x78,   /* Letter 't'          (d, e, f, g)           */
        /* 25 */     0x20,   /* Symbol '\'' (Apos)  (f)                    */
        /* 26 */     0x79,   /* Letter 'E'          (a, d, e, f, g)        */
    };

    /* Dynamic boundary check adapting to total array size */
    if (num >= (sizeof(segments) / sizeof(segments[0])))
    {
        return 0x00;
    }

    return decimalPoint ? (segments[num] | 0x80) : segments[num];
}

