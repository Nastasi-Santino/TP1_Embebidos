/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "serial_out.h"
#include "gpio.h"
#include "pisr.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PIN_SERIAL      PORTNUM2PIN(PB, 11) /**< Serial Data input pin (PB11) */
#define PIN_SCLK        PORTNUM2PIN(PB, 2)  /**< Shift Register Clock pin (PB2) */
#define PIN_RCLK        PORTNUM2PIN(PB, 3)  /**< Storage Register / Latch Clock pin (PB3) */
#define PIN_OE          PORTNUM2PIN(PB, 10) /**< Output Enable pin (Active Low) (PB10) */

#define SCLK_TICKS      PISR_US_TO_TICKS(SER_CLK_PERIOD_US) /**< Clock pulse period in timer ticks */

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Periodic timer callback generator for software clock synchronization.
 */
void serCLK_geneator(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool serCLK; /**< Software clock state toggle flag */

/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Configures shift register control GPIOs and registers clock generation timer.
 * @return True if GPIO configuration and periodic timer registration succeeded, false otherwise.
 */
bool serial_out_INIT(void)
{
    /* Configure control lines as digital outputs */
    gpioMode(PIN_SERIAL, OUTPUT);
    gpioMode(PIN_SCLK, OUTPUT);
    gpioMode(PIN_RCLK, OUTPUT);
    gpioMode(PIN_OE, OUTPUT);

    /* Initialize control lines to default inactive states */
    gpioWrite(PIN_SERIAL, LOW);
    gpioWrite(PIN_SCLK, LOW);
    gpioWrite(PIN_RCLK, LOW);
    gpioWrite(PIN_OE, LOW);

    /* Attach periodic timer callback to drive software clock toggling */
    if(!pisrRegister(serCLK_geneator, SCLK_TICKS))
    {
        return false;
    }
    return true;
}

/**
 * @brief Serializes segment, column selection, and LED status payload into shift registers.
 * @param seg 7-segment digit bitmask payload byte.
 * @param sel Target active column/digit index bitmask.
 * @param status Indicator LED output combination mask.
 */
void serial_out(uint8_t seg, uint8_t sel, uint8_t status)
{
    bool data;
    uint8_t counter = 0;
    bool first = true;
    bool flag = true;

    /* Loop until 14 data bits are clocked out into shift registers */
    while(counter < 14 || !flag)
    {
        if(!serCLK)
        {
            first = false;
            if(flag)
            {
                /* Extract bit payload sequentially based on bit counter offset */
                if(counter < 2)
                {
                    /* Bits 0-1: Shift out digit selection mask (2 bits MSB) */
                    data = (sel & 0x02) == 0x02;
                    sel <<= 1;
                } else if(counter < 4)
                {
                    /* Bits 2-3: Shift out status LED mask (2 bits MSB) */
                    data = (status & 0x02) == 0x02;
                    status <<= 1;
                } else if(counter != 5 && counter != 13)
                {
                    /* Bits 4, 6-12: Shift out 7-segment segment data (LSB first) */
                    data = (seg & 0x01) == 0x01;
                    seg >>= 1;
                } else
                {
                    /* Dummy padding bits inserted at indices 5 and 13 */
                    data = 0;
                }

                /* Write current data bit to serial line and pull clock line LOW */
                gpioWrite(PIN_SERIAL, data);
                gpioWrite(PIN_SCLK, 0);
                counter++;
                flag = false;
            }
        } else
        {
            /* On rising edge of serCLK, drive SCLK HIGH to sample bit into shift register */
            if(!first && !flag)
            {
                gpioWrite(PIN_SCLK, 1);
                flag = true;
            }
        }
    }

    /* Disable outputs and pulse Latch Clock (RCLK) to transfer data to output register */
    gpioWrite(PIN_OE, HIGH);  /* Turn off outputs during latching to prevent ghosting */
    gpioWrite(PIN_RCLK, HIGH); /* Pulse register clock HIGH to latch transferred bits */
    flag = true;

    /* Delay loop waiting for 3 software clock transitions to ensure latch hold time */
    while(counter < 17)
    {
        if(serCLK)
        {
            if(flag)
            {
                counter++;
                flag = false;
            }
        } else
        {
            flag = true;
        }
    }

    /* Re-enable display output and reset clock/latch lines */
    gpioWrite(PIN_OE, LOW);   /* Enable display output (Active Low OE) */
    gpioWrite(PIN_SCLK, 0);
    gpioWrite(PIN_RCLK, LOW);
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINES & LOCAL FUNCTIONS
 ******************************************************************************/

/**
 * @brief Periodic PISR timer callback toggling the software clock state.
 */
void serCLK_geneator(void)
{
    serCLK = !serCLK;
}