/***************************************************************************//**
  @file     pisr.c
  @brief    Periodic Interrupt (PISR) driver
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "pisr.h"

#include "hardware.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SYSTICK_LOAD_INIT   ((__CORE_CLOCK__/1000000U*PISR_TICK_US) - 1U)


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
    unsigned int count;
    unsigned int period;
    pisr_callback_t callback;
} pisr_t;


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static pisr_t pisr[PISR_CANT];
static unsigned int pisr_counter=0;



/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



bool pisrRegister (pisr_callback_t fun, unsigned int period)
{
    if (fun == (void *)0) {
        return false;
    }

    if (period == 0){
    	return false;
    }

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
    }
    
    pisr[pisr_counter].count = 0;
    pisr[pisr_counter].period = period;
    pisr[pisr_counter].callback = fun;
    ++pisr_counter;

    return true;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void SysTick_Handler (void)
{
	for(int i = 0; i < pisr_counter; i++)
	{
		pisr[i].count++;

		if(pisr[i].count >= pisr[i].period)
		{
			pisr[i].callback();
			pisr[i].count = 0;
		}
	}
}


/******************************************************************************/
