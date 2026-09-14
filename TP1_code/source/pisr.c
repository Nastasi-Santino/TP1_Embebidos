/***************************************************************************//**
  @file     pisr.c
  @brief    Periodic Interrupt (PISR) driver implementation using ARM SysTick
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "pisr.h"
#include "hardware.h"

#define DEBUG_PISR

#ifdef DEBUG_PISR
// Test point to measure pisr time
#include "gpio.h"
#include "board.h"
#endif

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/**
 * @brief SysTick reload value calculation based on core clock frequency and target tick time.
 */
#define SYSTICK_LOAD_INIT   ((__CORE_CLOCK__ / 1000000U * PISR_TICK_US) - 1U)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/**
 * @brief Structure representing a registered periodic interrupt callback descriptor.
 */
typedef struct {
    unsigned int count;       /**< Current tick counter for callback period tracking */
    unsigned int period;      /**< Total ticks required between callback executions */
    pisr_callback_t callback; /**< Function pointer to the periodic callback */
} pisr_t;

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static pisr_t pisr[PISR_CANT];       /**< Registry array of periodic callback slots */
static unsigned int pisr_counter = 0; /**< Total number of registered callbacks */

/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Registers a function to be executed periodically and initializes SysTick on first call.
 * @param fun Function pointer to callback routine.
 * @param period Callback period expressed in PISR ticks.
 * @return True if registration was successful, false if inputs are invalid or limits exceeded.
 */
bool pisrRegister (pisr_callback_t fun, unsigned int period)
{
    /* Parameter validation */
    if (fun == (void *)0) {
        return false;
    }

    if (period == 0){
        return false;
    }

    /* Lazy initialization of SysTick timer hardware upon first registration */
    static bool yaInit = false;
    if (!yaInit)
    {   
        yaInit = true;
        SysTick->CTRL = 0;
        SysTick->LOAD = SYSTICK_LOAD_INIT;
        SysTick->VAL  = 0;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                        SysTick_CTRL_TICKINT_Msk   |
                        SysTick_CTRL_ENABLE_Msk;

#ifdef DEBUG_PISR
        gpioMode(PIN_TEST_POINT_PISR, OUTPUT);
        gpioWrite(PIN_TEST_POINT_PISR, LOW);
#endif
    }
    
    /* Store callback entry parameters in active array */
    pisr[pisr_counter].count = 0;
    pisr[pisr_counter].period = period;
    pisr[pisr_counter].callback = fun;
    ++pisr_counter;

    return true;
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINES & LOCAL FUNCTIONS
 ******************************************************************************/

/**
 * @brief SysTick hardware Interrupt Handler executing registered periodic callbacks.
 * @details Iterates through all registered callback structures, increments tick counters,
 *          and triggers the function pointer upon period match.
 */
void SysTick_Handler (void)
{
#ifdef DEBUG_PISR
	gpioWrite(PIN_TEST_POINT_PISR, HIGH);
#endif
    for(int i = 0; i < pisr_counter; i++)
    {
        pisr[i].count++;

        if(pisr[i].count >= pisr[i].period)
        {
            pisr[i].callback();
            pisr[i].count = 0; /* Reset tick count after execution */
        }
    }
#ifdef DEBUG_PISR
    gpioWrite(PIN_TEST_POINT_PISR, LOW);
#endif
}
