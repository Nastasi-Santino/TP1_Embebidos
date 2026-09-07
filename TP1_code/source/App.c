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
void changeBrightness(bool dir);
void adminMenu(bool dir);
void changeIdMenuAdmin(bool dir);
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
	ASKING_PASSWORD,
	WAITING_PASSWORD,
	OPENING,
	WRONG_PASSWORD,
	CHANGING_PASSWORD,
	BRIGHTNESS,
	ADMIN_MODE
};

static user_t users[10];
static uint8_t users_cant;
static uint8_t active_user;
static uint8_t password_tries;
static bool first_in_state = true;
static bool adding_user = false;

static uint8_t admin_sub_mode;

static uint8_t state;
static uint8_t prev_state;
static uint8_t row;
static uint8_t brightness;

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
	brightness = HUNDRED_PERCENT_BRIGTHNESS;

	users[0] = (user_t){
	    .id = {6, 0, 3, 1, 6, 7, 0, 9},
	    .password = {0, 0, 0, 0, 0},
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

	users_cant = 2;
}


static uint8_t good[4]   = {G, o, o, d};
static uint8_t wrong[3]  = {X, X, X};
static uint8_t id_msg[4] = {GUION, I, d, GUION};
static uint8_t id_nF[4]  = {I, d, n, F};
static uint8_t password_msg[4] = {P, S, S, d};
static uint8_t cant[4] = {C, a, n, t};
static uint8_t ids[4] = {I, d, APOSTROFE,S};
static uint8_t add[3] = {a, d, d};
static uint8_t dlt[3] = {d, l, t};
static uint8_t exit[4] = {E, X, I, t};

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
			if(!timer_counting() || first_in_state == true)
			{
				start_timer_ms(2000);
				first_in_state = false;
			}
		}
		break;
	case WAITING_ID:
	case SHOWING_ID:
		length = id_counter;
		data = id;
		status = ONLY_FIRST_LED;
		mode = (state == SHOWING_ID) ? COMPLETE : EDITING;
		if(timer_finished()){
			state = ASKING_ID;
			id_counter = 0;
			password_counter = 0;
		} else
		{
			if(!timer_counting() || first_in_state == true)
			{
				start_timer_ms(20000);
				first_in_state = false;
			}
		}
		break;
	case ID_NOT_FOUND:
		length = 4;
		data = id_nF;
		status = 0;
		mode = COMPLETE;
		if(timer_finished())
		{
			state = WAITING_ID;
		} else
		{
			if(!timer_counting() || first_in_state == true)
			{
				start_timer_ms(2000);
				first_in_state = false;
			}
		}
		break;
	case ASKING_PASSWORD:
		length = 4;
		data = password_msg;
		status = FIRST_AND_SECOND_LED;
		mode = COMPLETE;
		if(timer_finished())
		{
			state = WAITING_PASSWORD;
		} else {
			if(!timer_counting() || first_in_state == true)
			{
				start_timer_ms(2000);
				first_in_state = false;
			}
		}
		break;
	case WAITING_PASSWORD:
	case CHANGING_PASSWORD:
		length = password_counter;
		data = password;
		status = FIRST_AND_SECOND_LED;
		mode = EDITING;
		if(timer_finished()){
			state = ASKING_ID;
			id_counter = 0;
			password_counter = 0;
		} else
		{
			if(!timer_counting() || first_in_state == true)
			{
				start_timer_ms(20000);
				first_in_state = false;
			}
		}
		break;
	case OPENING:
		length = 4;
		data = good;
		status = ALL_LEDS_ON;
		mode = COMPLETE;
		if(timer_finished())
		{
			id_counter = 0;
			password_counter = 0;
			state = ASKING_ID;
		} else {
			if(!timer_counting() || first_in_state == true)
			{
				start_timer_ms(5000);
				first_in_state = false;
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
			if(!timer_counting() || first_in_state == true)
			{
				start_timer_ms(1000);
				first_in_state = false;
			}
		}
		break;
	case BRIGHTNESS:
		length = (prev_state == WAITING_ID) ? id_counter : password_counter;
		data = (prev_state == WAITING_ID) ? id : password;
		status = FIRST_AND_THIRD_LED;
		mode = EDITING;
		private = (prev_state == WAITING_PASSWORD) ? true : false;
		setBrightness(brightness);
		break;
	case ADMIN_MODE:
		if(admin_sub_mode == 0)
		{
			length = 4;
			data = cant;
			status = SECOND_AND_THIRD_LED;
			mode = COMPLETE;
		} else if (admin_sub_mode == 1)
		{
			length = 4;
			data = ids;
			status = SECOND_AND_THIRD_LED;
			mode = COMPLETE;
		} else if(admin_sub_mode == 2)
		{
			length = 3;
			data = add;
			status = SECOND_AND_THIRD_LED;
			mode = COMPLETE;
		} else if(admin_sub_mode == 3)
		{
			length = 3;
			data = dlt;
			status = SECOND_AND_THIRD_LED;
			mode = COMPLETE;
		} else if(admin_sub_mode == 4)
		{
			length = 4;
			data = exit;
			status = SECOND_AND_THIRD_LED;
			mode = COMPLETE;
		} else if(admin_sub_mode == 5)
		{
			length = 1;
			data = &users_cant;
			status = SECOND_AND_THIRD_LED;
			mode = COMPLETE;
		} else if(admin_sub_mode == 6 || admin_sub_mode == 7)
		{
			length = 0;
			status = SECOND_AND_THIRD_LED;
			mode = EDITING;
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
		} else if(state == WAITING_PASSWORD || state == CHANGING_PASSWORD)
		{
			changeSelection(encoderDir(), password_counter == PASSWORD_MAX_LENGHT);
		} else if(state == BRIGHTNESS)
		{
			changeBrightness(encoderDir());
		} else if(state == ADMIN_MODE)
		{
			if(admin_sub_mode < 5)
			{
				adminMenu(encoderDir());
			} else if(admin_sub_mode == 6 || admin_sub_mode == 7)
			{
				changeIdMenuAdmin(encoderDir());
			}

		}
	}

	static bool button_pressed_flag = 0;
	if(buttonPressed())
	{
		if(!button_pressed_flag)
		{
			button_pressed_flag = 1;
			if(state == WAITING_ID || state == WAITING_PASSWORD || state == CHANGING_PASSWORD)
			{
				selectionEntered();
				reset_timer();
			} else if(state == SHOWING_ID)
			{
				if(prev_state == ADMIN_MODE)
				{
					state = ADMIN_MODE;
					prev_state = SHOWING_ID;
					row = 0;
				} else
				{
					state = ASKING_PASSWORD;
					selection = 0;
					row = 0;
					first_in_state = true;
				}

			} else if(state == BRIGHTNESS)
			{
				state = prev_state;
				first_in_state = true;
			} else if(state == ADMIN_MODE)
			{
				if(admin_sub_mode == 0)
				{
					admin_sub_mode = 5;
				} else if(admin_sub_mode == 1)
				{
					admin_sub_mode = 6;
					if(users_cant != 0)
					{
						selection = 1;
					} else
					{
						selection = E;
					}
				} else if(admin_sub_mode == 2){
					adding_user = true;
					id_counter = 0;
					password_counter = 0;
					selection = 0;
					state = ASKING_ID;
				} else if(admin_sub_mode == 3)
				{
					admin_sub_mode = 7;
					if(users_cant != 0)
					{
						selection = 1;
					} else
					{
						selection = E;
					}

				} else if(admin_sub_mode == 4)
				{
					id_counter = 0;
					password_counter = 0;
					selection = 0;
					state = ASKING_ID;
				}else if(admin_sub_mode == 5)
				{
					admin_sub_mode = 0;
				}else if(admin_sub_mode == 6)
				{
					if(selection == E)
					{
						admin_sub_mode = 1;
					} else
					{
						for(int i = 0; i < ID_LENGTH; i++)
						{
							id[i] = users[selection].id[i];
						}
						id_counter = ID_LENGTH;
						state = SHOWING_ID;
						prev_state = ADMIN_MODE;
					}

				} else if (admin_sub_mode == 7)
				{
					if(selection == E)
					{
						admin_sub_mode = 3;
					} else
					{
						for(int i = selection; i < users_cant; i++)
						{
							users[i] = users[i+1];
						}
						users_cant--;
						admin_sub_mode = 3;
					}
				}
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

void changeIdMenuAdmin(bool dir)
{
	uint8_t min = 1;
	uint8_t max;
	if(users_cant == 0)
	{
		min = E;
		max = E;
	} else
	{
		min = E;
		max = users_cant;
	}


	if(dir == IS_RIGHT)
	{
		if(selection == max)
		{
			selection = E;
		} else if(selection == E)
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
			selection = E;
		} else if(selection == E)
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
	} else if(state == WAITING_PASSWORD || state == CHANGING_PASSWORD)
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
	} else if(selection == 12)
	{
		prev_state = state;
		state = BRIGHTNESS;
		first_in_state = true;
	}else if(selection == 13)
	{
		if(state == WAITING_PASSWORD)
		{
			if(matchPassword())
			{
				password_counter = 0;
				state = CHANGING_PASSWORD;
				first_in_state = true;
			} else
			{
			password_tries++;
			password_counter = 0;
			state = WRONG_PASSWORD;
			first_in_state = true;
			}
		}
	}else if(selection == 14)
	{
		id_counter = 0;
		password_counter = 0;
		state = WAITING_ID;
		first_in_state = true;
	} else if(selection == 15)
	{
		if(state == WAITING_ID)
		{
			if(checkId() || adding_user)
			{
				state = SHOWING_ID;
				first_in_state = true;
			} else
			{
				id_counter = 0;
				state = ID_NOT_FOUND;
				first_in_state = true;
			}
		} else if(state == WAITING_PASSWORD)
		{
			if(matchPassword() || adding_user)
			{
				if(active_user == 0 && !adding_user)
				{
					state = ADMIN_MODE;
					selection = 0;
					first_in_state = true;
				} else if(adding_user)
				{
					for(int i = 0; i < ID_LENGTH; i++)
					{
						users[users_cant + 1].id[i] = id[i];
						if(i < password_counter)
						{
							users[users_cant + 1].password[i] = password[i];
						}
					}
					users[users_cant + 1].password_length = password_counter;
					users_cant++;
					adding_user = false;
					state = OPENING;
					id_counter = 0;
					password_counter = 0;
					admin_sub_mode = 0;
				} else
				{
					state = OPENING;
					first_in_state = true;
				}

			} else
			{
				password_tries++;
				password_counter = 0;
				state = WRONG_PASSWORD;
				first_in_state = true;
			}
		} else if(state == CHANGING_PASSWORD)
		{
			for(int i = 0; i < password_counter; i++)
			{
				users[active_user].password[i] = password[i];
			}
			users[active_user].password_length = password_counter;
			state = OPENING;
			id_counter = 0;
			password_counter = 0;
			first_in_state = true;
			selection = 0;
		}
	}

	if(state != BRIGHTNESS)
	{
		if((*counter) < max)
			{
				selection = 0;
			} else
			{
				selection = 15;
			}
	}

}


bool checkId(void)
{
	bool existing_id = false;
	for(int i = 0; i <= users_cant; i++)
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

void changeBrightness(bool dir)
{
	if(dir == IS_RIGHT)
	{
		if(brightness == HUNDRED_PERCENT_BRIGTHNESS)
		{
			brightness = HUNDRED_PERCENT_BRIGTHNESS;
		} else
		{
			brightness++;
		}
	} else
	{
		if(brightness == TEN_PERCENT_BRIGTHNESS)
		{
			brightness = TEN_PERCENT_BRIGTHNESS;
		} else
		{
			brightness--;
		}
	}
}

void adminMenu(bool dir)
{
	if(dir == IS_RIGHT)
	{
		if(admin_sub_mode == 4)
		{
			admin_sub_mode = 0;
		} else
		{
			admin_sub_mode++;
		}
	} else
	{
		if(admin_sub_mode == 0)
		{
			admin_sub_mode = 4;
		} else
		{
			admin_sub_mode--;
		}
	}
}

/*******************************************************************************
 ******************************************************************************/
