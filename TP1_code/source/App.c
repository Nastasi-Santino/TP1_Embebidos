/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "board.h"
#include "gpio.h"
#include "pisr.h"
#include "card_reader.h"
#include "encoder.h"
#include "hardware.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define COLOR GREEN
#define SWITCH 2

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define PIN_RGB CONCAT(PIN_LED_, COLOR)
#define PIN_SW CONCAT(PIN_SW, SWITCH)

#define SELECTION_MODES	12
#define ID_LENGHT 8
#define PASSWORD_MIN_LENGHT 4
#define PASSWORD_MAX_LENGHT 5

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void changeSelection(bool dir);
void selectionEntered(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool waiting_id;
static uint8_t id[8];
static uint8_t id_counter;

static bool waiting_password;
static uint8_t password[5];
static uint8_t password_counter;

static uint8_t selection;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	card_reader_INIT(PIN_CR_ENABLE, PIN_CR_CLOCK, PIN_CR_DATA);
	encoder_INIT(PIN_SW_ENCODER, PIN_A_ENCODER, PIN_B_ENCODER);

}



/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	if(encoderMoved())
	{
		changeSelection(encoderDir());
	}

	if(buttonPressed())
	{
		selectionEntered();
	}
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void changeSelection(bool dir)
{
	if(dir == IS_RIGHT)
	{
		if(selection == SELECTION_MODES - 1)
		{
			selection = 0;
		} else
		{
			selection++;
		}
	} else
	{
		if(selection == 0)
		{
			selection = SELECTION_MODES - 1;
		} else
		{
			selection--;
		}
	}
}


void selectionEntered(void)
{
	if(selection >= 0 && selection <= 9)
	{
		if(waiting_id)
		{
			if(id_counter < ID_LENGHT)
			{
				id[id_counter++] = selection;
			}
		} else if(waiting_password)
		{
			if(password_counter < PASSWORD_MAX_LENGHT)
			{
				password[password_counter++] = selection;
			}
		}
	}
}

/*******************************************************************************
 ******************************************************************************/
