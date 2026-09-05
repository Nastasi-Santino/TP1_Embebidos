/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "board.h"
#include "card_reader.h"
#include "encoder.h"
#include "card_decoder.h"
#include "display.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SELECTION_MODES	16
#define ID_LENGHT 8
#define PASSWORD_MIN_LENGHT 4
#define PASSWORD_MAX_LENGHT 5

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void changeSelection(bool dir, bool complete);
void selectionEntered(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

enum
{
	WAITING_ID,
	SHOWING_ID,
	WAITING_PASSWORD,
	OPENING,
	CHANGING_PASSWORD,
	BRIGHTNESS
};

static uint8_t state;

static uint8_t row;

static uint8_t id[8];
static uint8_t id_counter;

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

	display_INIT();

	state = WAITING_ID;
}



/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{

	uint8_t mode = (state == SHOWING_ID) ? COMPLETE : EDITING;
	bool private = (state == WAITING_PASSWORD) ? true : false;
	uint8_t length;
	uint8_t * data;

	if(state == WAITING_ID || state == SHOWING_ID)
	{
		length = id_counter;
		data = id;
	} else if(state == WAITING_PASSWORD)
	{
		length = password_counter;
		data = password;
	}
	print(data, length , selection,
			mode, private, row, 0x00);

	static track2_card_t card;
	if(state == WAITING_ID  && data_ready())
	{
		if(card_decode_track2(get_data(), get_data_length(), &card)){
			if(card.pan_length >= 8)
			{
				for(int i = 0; i < 8; i++)
				{
					id[i] = card.pan[i];
					id_counter = 8;
				}
				state++;
			}
		}
	}

	if(encoderMoved())
	{
		if(state == WAITING_ID)
		{
			changeSelection(encoderDir(), id_counter == ID_LENGHT);
		} else if(state == SHOWING_ID)
		{
			row = (row + 1) & 0x01;
		} else if(state == WAITING_PASSWORD)
		{
			changeSelection(encoderDir(), password_counter == PASSWORD_MAX_LENGHT);
		}
	}

	static bool button_pressed_flag = 0;
	if(buttonPressed())
	{
		if(!button_pressed_flag)
		{
			button_pressed_flag = 1;
			if(state == WAITING_ID || state == WAITING_PASSWORD)
			{
				selectionEntered();
			} else if(state == SHOWING_ID)
			{
				state++;
				selection = 0;
			}
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

void changeSelection(bool dir, bool complete)
{
	uint8_t max = SELECTION_MODES - 2;
	uint8_t min = 0;

	if(complete)
	{
		max = SELECTION_MODES - 1;
		min = 10;
	}

	if(password_counter >= PASSWORD_MIN_LENGHT)
	{
		max = SELECTION_MODES - 1;
	}


	if(dir == IS_RIGHT)
	{
		if(selection == max)
		{
			selection = min;
		} else
		{
			selection++;
		}
	} else
	{
		if(selection == min)
		{
			selection = max;
		} else
		{
			selection--;
		}
	}
}


void selectionEntered(void)
{
	uint8_t * counter;
	uint8_t * data;
	uint8_t max;
	if(state == WAITING_ID)
	{
		counter = &id_counter;
		data = id;
		max = ID_LENGHT;
	} else if(state == WAITING_PASSWORD)
	{
		counter = &password_counter;
		data = password;
		max = PASSWORD_MAX_LENGHT;
	}

	if(selection >= 0 && selection <= 9)
	{
		if(*counter < max)
		{
			data[(*counter)++] = selection;
		}
	} else if(selection == 10)
	{
		if((*counter) != 0)
		{
			(*counter)--;
		}
	} else if(selection == 11)
	{
		(*counter) = 0;
	} else if(selection == 14)
	{
		(*counter) = 0;
		state = WAITING_ID;
	} else if(selection == 15)
	{
		state++;
	}

	if((*counter) < max)
	{
		selection = 0;
	} else
	{
		selection = 15;
	}
}


/*******************************************************************************
 ******************************************************************************/
