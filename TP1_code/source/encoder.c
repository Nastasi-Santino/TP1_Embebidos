/***************************************************************************//**
  @file     encoder.c
  @brief    Quadrature encoder and push-button driver implementation
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "encoder.h"
#include "gpio.h"
#include "pisr.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PIN_SW_ENCODER          PORTNUM2PIN(PC, 11) /**< Push-button switch input pin (PC11) */
#define PIN_A_ENCODER           PORTNUM2PIN(PB, 18) /**< Quadrature Channel A input pin (PB18) */
#define PIN_B_ENCODER           PORTNUM2PIN(PC, 10) /**< Quadrature Channel B input pin (PC10) */

#define BUTTON_PERIOD_MS        50U /**< Debounce polling interval for push-button in ms */
#define ENCODER_PERIOD_MS       3U  /**< Debounce confirmation window for encoder pulses in ms */
#define ENCODER_COUNTER         3   /**< Debounce tick count threshold for noise validation */

#define BUTTON_PERIOD_TICKS     PISR_MS_TO_TICKS(BUTTON_PERIOD_MS)
#define ENCODER_PERIOD_TICKS    PISR_MS_TO_TICKS(ENCODER_PERIOD_MS/ENCODER_COUNTER)

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Periodic PISR callback to sample and debounce push-button state.
 */
static void button_PISR(void);

/**
 * @brief Interrupt handler for falling edge on Quadrature Channel A.
 */
static void A_IRQHandler(void);

/**
 * @brief Interrupt handler for falling edge on Quadrature Channel B.
 */
static void B_IRQHandler(void);

/**
 * @brief Periodic PISR callback to debounce and process phase relationship of channels.
 */
static void encoder_PISR(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool counting_a;          /**< Flag indicating Channel A interrupt is undergoing debounce validation */
static bool counting_b;          /**< Flag indicating Channel B interrupt is undergoing debounce validation */
static bool a_active;            /**< Verified active low state for Channel A pulse */
static bool b_active;            /**< Verified active low state for Channel B pulse */

static bool buttonPressed_flag;  /**< Current debounced state of push-button (true = pressed) */
static bool encoderMoved_flag;   /**< One-shot flag set when valid encoder rotation is detected */
static bool encoderDir_flag;     /**< Direction of last detected encoder rotation (IS_LEFT / IS_RIGHT) */

/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Initializes encoder GPIO input pins, edge interrupts, and periodic sampling handlers.
 * @return True if GPIO IRQs and PISR callbacks were registered successfully, false otherwise.
 */
bool encoder_INIT(void)
{
    /* Configure encoder channels and push-button as digital inputs */
    gpioMode(PIN_SW_ENCODER, INPUT);
    gpioMode(PIN_A_ENCODER, INPUT);
    gpioMode(PIN_B_ENCODER, INPUT);

    /* Register falling edge interrupts on Channel A and Channel B */
    bool a_flag = gpioIRQ(PIN_A_ENCODER, GPIO_IRQ_MODE_FALLING_EDGE, A_IRQHandler);
    bool b_flag = gpioIRQ(PIN_B_ENCODER, GPIO_IRQ_MODE_FALLING_EDGE, B_IRQHandler);

    /* Register periodic PISR callbacks for button polling and encoder debouncing */
    bool button_flag = pisrRegister(button_PISR, BUTTON_PERIOD_TICKS);
    bool encoder_flag = pisrRegister(encoder_PISR, ENCODER_PERIOD_TICKS);

    if(!a_flag || !b_flag || !button_flag || !encoder_flag)
    {
        return false;
    }

    return true;
}

/**
 * @brief Polls current debounced push-button state.
 * @return True if button is currently pressed, false otherwise.
 */
bool buttonPressed(void)
{
    return buttonPressed_flag;
}

/**
 * @brief Checks if an encoder rotation occurred and automatically clears movement flag upon read.
 * @return True if encoder moved since last query, false otherwise.
 */
bool encoderMoved(void)
{
    if(encoderMoved_flag)
    {
        encoderMoved_flag = false; /* Clear flag on read (one-shot query) */
        return true;
    }

    return false;
}

/**
 * @brief Returns direction of the last detected encoder rotation.
 * @return IS_LEFT or IS_RIGHT.
 */
bool encoderDir(void)
{
    return encoderDir_flag;
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINES & LOCAL FUNCTIONS
 ******************************************************************************/

/**
 * @brief Periodic PISR callback for sampling push-button status (active LOW).
 */
static void button_PISR(void)
{
    if(gpioRead(PIN_SW_ENCODER))
    {
        buttonPressed_flag = 0;
    } else
    {
        buttonPressed_flag = 1;
    }
}

/**
 * @brief Interrupt handler triggered on falling edge of Channel A.
 */
static void A_IRQHandler(void)
{
    if(!counting_a)
    {
        counting_a = true;
    }
}

/**
 * @brief Interrupt handler triggered on falling edge of Channel B.
 */
static void B_IRQHandler(void)
{
    if(!counting_b)
    {
        counting_b = true;
    }
}

/**
 * @brief Periodic PISR tick handler validating quadrature signals and resolving rotation direction.
 */
static void encoder_PISR(void)
{
    static uint8_t counter_a = 0;
    static uint8_t counter_b = 0;

    /* Debounce validation sequence for Channel A */
    if(counting_a)
    {
        if(counter_a >= ENCODER_COUNTER)
        {
            if(!gpioRead(PIN_A_ENCODER))
            {
                a_active = true;
                if(b_active)
                {
                    encoderMoved_flag = true;
                    encoderDir_flag = IS_RIGHT;
                    a_active = false;
                    b_active = false;
                }
            } else
            {
                a_active = false;
            }
            counter_a = 0;
            counting_a = false;
        }
        counter_a++;
    }

    /* Debounce validation sequence for Channel B */
    if(counting_b)
    {
        if(counter_b >= ENCODER_COUNTER)
        {
            if(!gpioRead(PIN_B_ENCODER))
            {
                b_active = true;
                if(a_active)
                {
                    encoderMoved_flag = true;
                    encoderDir_flag = IS_LEFT;
                    a_active = false;
                    b_active = false;
                }
            } else
            {
                b_active = false;
            }
            counter_b = 0;
            counting_b = false;
        }
        counter_b++;
    }
}