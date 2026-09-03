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
#include "card_decoder.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define COLOR RED
#define SWITCH 2

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define PIN_RGB CONCAT(PIN_LED_, COLOR)
#define PIN_SW CONCAT(PIN_SW, SWITCH)

#define SELECTION_MODES	15
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
	card_reader_INIT();
	encoder_INIT();

	// DEBUG
	gpioMode(PIN_LED_RED, OUTPUT);
	//gpioMode(PIN_LED_GREEN, OUTPUT);
	//gpioMode(PIN_LED_BLUE, OUTPUT);

	gpioWrite(PIN_LED_RED, !LED_ACTIVE);
	//gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);
	//gpioWrite(PIN_LED_BLUE, !LED_ACTIVE);


}



/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	static track2_card_t tarjeta;
	if(data_ready())
	{
		if(card_decode_track2(get_data(), get_data_length(), &tarjeta)){
			gpioToggle(PIN_LED_RED);
		}
	}

	/*

	if(encoderMoved())
	{
		if(encoderDir() == IS_RIGHT)
		{
			gpioWrite(PIN_LED_RED, LED_ACTIVE);
			gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);
			gpioWrite(PIN_LED_BLUE, !LED_ACTIVE);
		} else
		{
			gpioWrite(PIN_LED_RED, !LED_ACTIVE);
			gpioWrite(PIN_LED_GREEN, LED_ACTIVE);
			gpioWrite(PIN_LED_BLUE, !LED_ACTIVE);
		}
	}

	static bool button_pressed_flag = 0;
	if(buttonPressed())
	{
		if(!button_pressed_flag)
		{
			button_pressed_flag = 1;
			gpioWrite(PIN_LED_RED, !LED_ACTIVE);
			gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);
			gpioWrite(PIN_LED_BLUE, LED_ACTIVE);
		} else
		{
			button_pressed_flag = 0;
		}
	}
	*/

	if(encoderMoved())
	{
		changeSelection(encoderDir());
	}

	static bool button_pressed_flag = 0;
	if(buttonPressed())
	{
		if(!button_pressed_flag)
		{
			button_pressed_flag = 1;
			selectionEntered();
		}
	} else
	{
		button_pressed_flag = 0;
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
