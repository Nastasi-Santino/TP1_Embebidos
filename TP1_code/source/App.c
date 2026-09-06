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
#include "timer.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SELECTION_MODES	16
#define ID_LENGTH 8
#define PASSWORD_MIN_LENGHT 4
#define PASSWORD_MAX_LENGHT 5
#define USERS_IN_SYSTEM		3
#define MAX_PASSWORD_TRIES	3

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void changeSelection(bool dir, bool complete);
void selectionEntered(void);
bool checkId(void);
bool matchPassword(void);
/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

typedef struct
{
    uint8_t id[8];
    uint8_t password[5];
    uint8_t password_length;
} user_t;

enum
{
	ASKING_ID,
	WAITING_ID,
	SHOWING_ID,
	ID_NOT_FOUND,
	WAITING_PASSWORD,
	OPENING,
	WRONG_PASSWORD,
	CHANGING_PASSWORD,
	BRIGHTNESS
};

static user_t users[3];
static uint8_t active_user;
static uint8_t password_tries;

static uint8_t state;

static uint8_t row;

static uint8_t id[8];
static uint8_t id_counter;

static uint8_t password[5];
static uint8_t password_counter;

static uint8_t selection;
static uint8_t status;

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
	timer_INIT();

	state = ASKING_ID;

	users[0] = (user_t){
	    .id = {6, 0, 3, 2, 6, 7, 0, 9},
	    .password = {6, 5, 1, 1},
		.password_length = 4
	};

	users[1] = (user_t){
	    .id = {4, 5, 4, 8, 3, 2, 0, 0},
	    .password = {1, 0, 2, 2, 9},
		.password_length = 5
	};

	users[2] = (user_t){
	    .id = {4, 0, 6, 6, 6, 3, 4, 1},
	    .password = {0, 4, 2, 8},
		.password_length = 4
	};
}


static uint8_t good[4]   = {G, o, o, d};
static uint8_t wrong[3]  = {X, X, X};
static uint8_t id_msg[4] = {GUION, I, d, GUION};
static uint8_t id_nF[4]  = {I, d, n, F};
/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{

	uint8_t mode;
	bool private = (state == WAITING_PASSWORD) ? true : false;
	uint8_t length;
	uint8_t * data;

	switch(state)
	{
	case ASKING_ID:
		length = 4;
		data = id_msg;
		status = 0;
		mode = COMPLETE;
		if(timer_finished())
		{
			state = WAITING_ID;
		} else {
			if(!timer_counting())
			{
				start_timer_ms(2000);
			}
		}
		break;
	case WAITING_ID:
	case SHOWING_ID:
		length = id_counter;
		data = id;
		status = SECOND_AND_THIRD_LED;
		mode = (state == SHOWING_ID) ? COMPLETE : EDITING;
		break;
	case ID_NOT_FOUND:
		length = 4;
		data = id_nF;
		status = 0;
		mode = COMPLETE;
		if(timer_finished())
		{
			state = WAITING_ID;
		} else {
			if(!timer_counting())
			{
				start_timer_ms(2000);
			}
		}
		break;
	case WAITING_PASSWORD:
		length = password_counter;
		data = password;
		status = 0;
		mode = EDITING;
		break;
	case OPENING:
		length = 4;
		data = good;
		status = 3;
		mode = COMPLETE;
		if(timer_finished())
		{
			state = ASKING_ID;
		} else {
			if(!timer_counting())
			{
				start_timer_ms(5000);
			}
		}
		break;
	case WRONG_PASSWORD:
		length = password_tries;
		data = wrong;
		status = 0;
		mode = COMPLETE;
		if(timer_finished())
		{
			if(password_tries < 3)
			{
				state = WAITING_PASSWORD;
			} else
			{
				state = ASKING_ID;
				id_counter = 0;
			}
		} else {
			if(!timer_counting())
			{
				start_timer_ms(1000);
			}
		}
		break;
	default:
		break;
	}


	print(data, length , selection,
			mode, private, row, status);

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
				if(checkId())
				{
					state = SHOWING_ID;
				} else
				{
					id_counter = 0;
					state = ID_NOT_FOUND;
				}
			}
		}
	}

	if(encoderMoved())
	{
		if(state == WAITING_ID)
		{
			changeSelection(encoderDir(), id_counter == ID_LENGTH);
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
				state = WAITING_PASSWORD;
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
		max = ID_LENGTH;
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
		id_counter = 0;
		password_counter = 0;
		state = WAITING_ID;
	} else if(selection == 15)
	{
		if(state == WAITING_ID)
		{
			if(checkId())
			{
				state = SHOWING_ID;
			} else
			{
				id_counter = 0;
			}
		} else if(state == WAITING_PASSWORD)
		{
			if(matchPassword())
			{
				state = OPENING;
			} else
			{
				password_tries++;
				password_counter = 0;
				state = WRONG_PASSWORD;
			}
		}
	}

	if((*counter) < max)
	{
		selection = 0;
	} else
	{
		selection = 15;
	}
}


bool checkId(void)
{
	bool existing_id = false;
	for(int i = 0; i < USERS_IN_SYSTEM; i++)
	{
		uint8_t j = 0;
		while((id[j] == users[i].id[j]) && (j < ID_LENGTH))
		{
			j++;
		}

		if(j == ID_LENGTH)
		{
			existing_id = true;
			active_user = i;
			break;
		}
	}

	return existing_id;
}

bool matchPassword(void)
{
	uint8_t m = 0;
	while((password[m] == users[active_user].password[m]) && (m < users[active_user].password_length))
	{
		m++;
	}

	return m == users[active_user].password_length;
}


/*******************************************************************************
 ******************************************************************************/
