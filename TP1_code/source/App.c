/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "board.h"        /* Hardware and pin configuration definitions */
#include "card_reader.h"  /* Magnetic card reader driver */
#include "encoder.h"      /* Rotary encoder driver for UI navigation */
#include "card_decoder.h" /* Parser for raw card data */
#include "display.h"      /* Display driver interface */
#include "timer.h"        /* System time management and delays */


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SELECTION_MODES      16  /**< Total number of selectable UI modes */
#define ID_LENGTH             8  /**< Required byte/digit length for user IDs */
#define PASSWORD_MIN_LENGHT   4  /**< Minimum password length limit */
#define PASSWORD_MAX_LENGHT   5  /**< Maximum password length limit */
#define USERS_IN_SYSTEM       3  /**< Total registered user capacity */
#define MAX_PASSWORD_TRIES    3  /**< Max failed login attempts allowed before lockout */

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Updates the menu selection index based on encoder rotation.
 * @param dir Rotation direction (true: clockwise / false: counter-clockwise).
 * @param complete Indicates if a full mechanical step/detent was completed.
 */
void changeSelection(bool dir, bool complete);

/**
 * @brief Handles user input confirmation (e.g., encoder button press).
 */
void selectionEntered(void);

/**
 * @brief Verifies if the entered ID exists in the user database.
 * @return True if valid ID found, false otherwise.
 */
bool checkId(void);

/**
 * @brief Checks if the entered password matches the active user's credentials.
 * @return True if password is correct, false otherwise.
 */
bool matchPassword(void);

/**
 * @brief Adjusts the display brightness step-by-step.
 * @param dir Direction to shift brightness (true: increase / false: decrease).
 */
void changeBrightness(bool dir);

/*******************************************************************************
 * PRIVATE DATA TYPES AND ENUMERATIONS
 ******************************************************************************/

/**
 * @brief Represents a single user profile stored in system memory.
 */
typedef struct
{
    uint8_t id[ID_LENGTH];                 /**< Array storing user ID digits */
    uint8_t password[PASSWORD_MAX_LENGHT]; /**< Array storing user password digits */
    uint8_t password_length;               /**< Actual length of user's password */
} user_t;

/**
 * @brief System Finite State Machine (FSM) states.
 */
enum
{
    ASKING_ID,          /**< Prompting user to swipe or enter ID */
    WAITING_ID,         /**< Processing/awaiting full ID input */
    SHOWING_ID,         /**< Displaying parsed ID on screen */
    ID_NOT_FOUND,       /**< Error state: Entered ID does not exist */
    ASKING_PASSWORD,    /**< Prompting user to input password */
    WAITING_PASSWORD,   /**< Processing/awaiting password submission */
    OPENING,            /**< Success state: Access granted, opening door/lock */
    WRONG_PASSWORD,     /**< Error state: Invalid password entered */
    CHANGING_PASSWORD,  /**< Password configuration mode */
    BRIGHTNESS          /**< Display brightness adjustment screen */
};

<<<<<<< HEAD
static user_t users[3];
static uint8_t active_user;
static uint8_t password_tries;
static bool first_in_state = true;

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
=======
>>>>>>> a03be315c002c876aecff0c54b3aeeff943375d6

/*******************************************************************************
 * STATIC / FILE-SCOPE VARIABLES
 ******************************************************************************/

/* User Database & Authentication Controls */
static user_t users[USERS_IN_SYSTEM];        /**< System database of registered users */
static uint8_t active_user;                  /**< Index of currently identified user */
static uint8_t password_tries;               /**< Consecutive failed password attempt counter */

/* State Machine Variables */
static uint8_t state;                        /**< Current system state */
static uint8_t prev_state;                   /**< Previous system state for UI navigation */

/* Display Settings */
static uint8_t row;                          /**< Target display line/row index */
static uint8_t brightness;                   /**< Current screen brightness level */

/* Input Buffers */
static uint8_t id[ID_LENGTH];                /**< Active buffer for incoming ID digits */
static uint8_t id_counter;                   /**< Count of received ID digits */

static uint8_t password[PASSWORD_MAX_LENGHT];/**< Active buffer for incoming password digits */
static uint8_t password_counter;             /**< Count of received password digits */

/* General UI & System Status */
static uint8_t selection;                    /**< Currently selected menu item index */
static uint8_t status;                       /**< System status and error flags */

/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Main system initialization routine called once at startup.
 * @details Configures peripherals, resets FSM state, and initializes user database.
 */
void App_Init (void)
{
    /* Initialize hardware drivers and peripherals */
    card_reader_INIT();
    encoder_INIT();

    display_INIT();
    timer_INIT();

    /* Set default system state and display settings */
    state = ASKING_ID;
    brightness = HUNDRED_PERCENT_BRIGTHNESS;

    /* Populate default user profiles (ID, Password, Password Length) */
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


/*******************************************************************************
 * DISPLAY TEXT BUFFERS & CONSTANTS
 ******************************************************************************/

static uint8_t good[4]         = {G, o, o, d};         /**< Display: "Good" (Access granted) */
static uint8_t wrong[3]        = {X, X, X};            /**< Display: "XXX" (Access denied / invalid password) */
static uint8_t id_msg[4]       = {GUION, I, d, GUION}; /**< Display: "-Id-" (ID prompt) */
static uint8_t id_nF[4]        = {I, d, n, F};         /**< Display: "IdnF" (ID not found error) */
static uint8_t password_msg[4] = {P, S, S, d};        /**< Display: "PSSd" (Password prompt) */

/**
 * @brief Main execution loop called continuously in an infinite main loop.
 * @details Handles system FSM state transitions, display updates, card reader inputs,
 *          and rotary encoder interactions.
 */
void App_Run (void)
{
    uint8_t mode;
    bool private = (state == WAITING_PASSWORD) ? true : false; /* Obfuscate input when typing password */
    uint8_t length;
    uint8_t * data;

    /* =========================================================================
     * FINITE STATE MACHINE (FSM)
     * ========================================================================= */
    switch(state)
    {
    /* State: Display initial ID prompt screen ("-Id-") */
    case ASKING_ID:
        length = 4;
        data = id_msg;
        status = 0;
        mode = COMPLETE;
        
        /* Non-blocking 2-second splash timer -> transition to ID input */
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

<<<<<<< HEAD

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
	default:
		break;
	}
=======
    /* States: User entering ID digits OR displaying valid swiped ID */
    case WAITING_ID:
    case SHOWING_ID:
        length = id_counter;
        data = id;
        status = SECOND_AND_THIRD_LED;
        mode = (state == SHOWING_ID) ? COMPLETE : EDITING;
        break;
>>>>>>> a03be315c002c876aecff0c54b3aeeff943375d6

    /* State: Error screen displayed when entered ID is not in database ("IdnF") */
    case ID_NOT_FOUND:
        length = 4;
        data = id_nF;
        status = 0;
        mode = COMPLETE;
        
        /* Hold error message for 2 seconds before returning to ID input */
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

    /* State: Prompt screen before password entry ("PSSd") */
    case ASKING_PASSWORD:
        length = 4;
        data = password_msg;
        status = 0;
        mode = COMPLETE;
        
        /* Hold prompt for 2 seconds before activating password entry mode */
        if(timer_finished())
        {
            state = WAITING_PASSWORD;
        } else {
            if(!timer_counting())
            {
                start_timer_ms(2000);
            }
        }
        break;

    /* State: Active password entry (inputs masked/hidden) */
    case WAITING_PASSWORD:
        length = password_counter;
        data = password;
        status = 0;
        mode = EDITING;
        break;

    /* State: Access Granted screen ("Good") and door unlocking action */
    case OPENING:
        length = 4;
        data = good;
        status = 3; /* Activate success LEDs / unlock relay */
        mode = COMPLETE;
        
        /* Hold door open for 5 seconds, then reset FSM to initial state */
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

<<<<<<< HEAD
	static bool button_pressed_flag = 0;
	if(buttonPressed())
	{
		if(!button_pressed_flag)
		{
			button_pressed_flag = 1;
			if(state == WAITING_ID || state == WAITING_PASSWORD)
			{
				selectionEntered();
				reset_timer();
			} else if(state == SHOWING_ID)
			{
				state = ASKING_PASSWORD;
				selection = 0;
				row = 0;
				first_in_state = true;
			} else if(state == BRIGHTNESS)
			{
				state = prev_state;
				first_in_state = true;
			}
		}
	} else
	{
		button_pressed_flag = 0;
	}
=======
    /* State: Access Denied screen ("XXX") with retry limit checking */
    case WRONG_PASSWORD:
        length = password_tries;
        data = wrong;
        status = 0;
        mode = COMPLETE;
        
        /* Hold error for 1 second, then evaluate remaining retry attempts */
        if(timer_finished())
        {
            if(password_tries < 3)
            {
                state = WAITING_PASSWORD; /* Allow another attempt */
            } else
            {
                state = ASKING_ID;        /* Exceeded limit: Lockout and reset */
                id_counter = 0;
            }
        } else {
            if(!timer_counting())
            {
                start_timer_ms(1000);
            }
        }
        break;

    /* State: Screen brightness setup overlay */
    case BRIGHTNESS:
        length = (prev_state == WAITING_ID) ? id_counter : password_counter;
        data = (prev_state == WAITING_ID) ? id : password;
        status = 0;
        mode = EDITING;
        private = (prev_state == WAITING_PASSWORD) ? true : false;
        setBrightness(brightness);
        break;

    default:
        break;
    }

    /* Refresh display output with updated state configuration */
    print(data, length , selection,
          mode, private, row, status);

    /* =========================================================================
     * CARD READER PROCESSING (Track 2 Decoding)
     * ========================================================================= */
    static track2_card_t card;
    if(state == WAITING_ID && data_ready())
    {
        /* Attempt to decode magnetic track data when available */
        if(card_decode_track2(get_data(), get_data_length(), &card)){
            if(card.pan_length >= 8)
            {
                /* Extract first 8 digits of PAN as account ID */
                for(int i = 0; i < 8; i++)
                {
                    id[i] = card.pan[i];
                    id_counter = 8;
                }
                
                /* Validate extracted ID against stored user database */
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

    /* =========================================================================
     * USER INPUT: ROTARY ENCODER NAVIGATION
     * ========================================================================= */
    if(encoderMoved())
    {
        if(state == WAITING_ID)
        {
            changeSelection(encoderDir(), id_counter == ID_LENGTH);
        } else if(state == SHOWING_ID)
        {
            row = (row + 1) & 0x01; /* Toggle row index (0 or 1) */
        } else if(state == WAITING_PASSWORD)
        {
            changeSelection(encoderDir(), password_counter == PASSWORD_MAX_LENGHT);
        } else if(state == BRIGHTNESS)
        {
            changeBrightness(encoderDir());
        }
    }

    /* =========================================================================
     * USER INPUT: ENCODER PUSH BUTTON (Falling Edge Detection)
     * ========================================================================= */
    static bool button_pressed_flag = 0;
    if(buttonPressed())
    {
        if(!button_pressed_flag)
        {
            button_pressed_flag = 1; /* Register button press event */
            
            if(state == WAITING_ID || state == WAITING_PASSWORD)
            {
                selectionEntered();
            } else if(state == SHOWING_ID)
            {
                state = ASKING_PASSWORD;
                selection = 0;
                row = 0;
            } else if(state == BRIGHTNESS)
            {
                state = prev_state; /* Exit brightness adjustment */
            }
        }
    } else
    {
        button_pressed_flag = 0; /* Reset flag on button release */
    }
>>>>>>> a03be315c002c876aecff0c54b3aeeff943375d6

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
	} else if(selection == 12)
	{
		prev_state = state;
		state = BRIGHTNESS;
		first_in_state = true;
	}else if(selection == 13)
	{
		if(state == WAITING_PASSWORD)
		{
			password_counter = 0;
			state = CHANGING_PASSWORD;
			first_in_state = true;
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
			if(checkId())
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
			if(matchPassword())
			{
				state = OPENING;
				first_in_state = true;
			} else
			{
				password_tries++;
				password_counter = 0;
				state = WRONG_PASSWORD;
				first_in_state = true;
			}
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

/*******************************************************************************
 ******************************************************************************/
