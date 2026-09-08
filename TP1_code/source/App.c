/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "board.h"        /* Hardware and pin configuration definitions */
#include "card_reader.h"  /* Magnetic/RFID card reader driver */
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
#define USERS_IN_SYSTEM       3  /**< Total registered user capacity limit */
#define MAX_PASSWORD_TRIES    3  /**< Max failed login attempts allowed before lockout */

/* Keypad / Encoder Action Mappings for selectionEntered */
#define KEY_BACKSPACE        10  /**< Action: Delete last digit */
#define KEY_CLEAR            11  /**< Action: Clear active buffer */
#define KEY_BRIGHTNESS       12  /**< Action: Open brightness adjustment */
#define KEY_CHANGE_PASS      13  /**< Action: Initiate password change */
#define KEY_CANCEL           14  /**< Action: Cancel and return to start */
#define KEY_ENTER            15  /**< Action: Confirm/Submit input */

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
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

/**
 * @brief Cycles through admin menu options based on encoder direction.
 * @param dir Direction of rotation (true: next / false: previous).
 */
void adminMenu(bool dir);

/**
 * @brief Navigates the stored user ID list within the admin menu.
 * @param dir Direction of rotation (true: next ID / false: previous ID).
 */
void changeIdMenuAdmin(bool dir);

/**
 * @brief Helper function to start state timeouts cleanly without code duplication.
 * @param ms Delay duration in milliseconds.
 */
static void start_state_timer(uint32_t ms);

/*******************************************************************************
 * PRIVATE DATA TYPES AND ENUMERATIONS
 ******************************************************************************/

/**
 * @brief Represents a single user profile stored in system memory.
 */
typedef struct
{
    uint8_t id[8];          /**< Array storing user ID digits */
    uint8_t password[5];    /**< Array storing user password digits */
    uint8_t password_length;/**< Actual length of user's password */
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
    BRIGHTNESS,         /**< Display brightness adjustment screen */
    ADMIN_MODE          /**< System administration menu screen */
};

/**
 * @brief Admin menu sub-modes for navigation and interaction screens.
 */
typedef enum
{
    ADMIN_SUB_CANT = 0,     /**< Menu option: "Cant" (User count option) */
    ADMIN_SUB_IDS,          /**< Menu option: "Id's" (View IDs option) */
    ADMIN_SUB_ADD,          /**< Menu option: "add" (Add user option) */
    ADMIN_SUB_DLT,          /**< Menu option: "dlt" (Delete user option) */
    ADMIN_SUB_EXIT,         /**< Menu option: "EXIt" (Exit admin menu) */
    ADMIN_SUB_SHOW_CANT,    /**< Display screen: Show total user count value */
    ADMIN_SUB_SELECT_ID,    /**< Selection screen: View specific user ID */
    ADMIN_SUB_DELETE_ID     /**< Selection screen: Choose user ID to delete */
} admin_submode_t;

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/* User Database & Administration Flags */
static user_t users[10];                     /**< System database of registered users (up to 10) */
static uint8_t users_cant;                   /**< Current count of registered users in system */
static uint8_t active_user;                  /**< Index of currently identified user */
static uint8_t password_tries;               /**< Consecutive failed password attempt counter */
static bool first_in_state = true;           /**< Flag indicating first entry into an FSM state */
static bool adding_user = false;             /**< Flag indicating admin user creation mode */

static uint8_t admin_sub_mode;               /**< Active sub-screen/item within Admin Mode */

/* State Machine Control Variables */
static uint8_t state;                        /**< Current system state */
static uint8_t prev_state;                   /**< Previous system state for UI navigation */

/* Display Control Settings */
static uint8_t row;                          /**< Target display line/row index */
static uint8_t brightness;                   /**< Current screen brightness level */

/* Active Input Buffers */
static uint8_t id[8];                        /**< Active buffer for incoming ID digits */
static uint8_t id_counter;                   /**< Count of received ID digits */

static uint8_t password[5];                  /**< Active buffer for incoming password digits */
static uint8_t password_counter;             /**< Count of received password digits */

/* General UI & System Status Flags */
static uint8_t selection;                    /**< Currently selected menu item index */
static uint8_t status;                       /**< System status and LED indicator flags */

/*******************************************************************************
 * DISPLAY TEXT BUFFERS & CONSTANTS
 ******************************************************************************/

static uint8_t good[4]         = {G, o, o, d};         /**< Display: "Good" (Access granted) */
static uint8_t wrong[3]        = {X, X, X};            /**< Display: "XXX" (Access denied) */
static uint8_t id_msg[4]       = {GUION, I, d, GUION}; /**< Display: "-Id-" (ID prompt) */
static uint8_t id_nF[4]        = {I, d, n, F};         /**< Display: "IdnF" (ID not found) */
static uint8_t password_msg[4] = {P, S, S, d};        /**< Display: "PSSd" (Password prompt) */
static uint8_t cant[4]         = {C, a, n, t};         /**< Display: "Cant" (User count menu item) */
static uint8_t ids[4]          = {I, d, APOSTROFE,S};  /**< Display: "Id's" (View IDs menu item) */
static uint8_t add[3]          = {a, d, d};            /**< Display: "add" (Add user menu item) */
static uint8_t dlt[3]          = {d, l, t};            /**< Display: "dlt" (Delete user menu item) */
static uint8_t exit[4]         = {E, X, I, t};         /**< Display: "EXIt" (Exit admin menu item) */


/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Main system initialization routine called once at startup.
 * @details Configures peripherals, resets FSM state, and populates default users.
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

    /* Populate default user database (ID, Password, Password Length) */
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

    users_cant = 2; /* Initial user count */
}


/**
 * @brief Main execution loop called continuously in an infinite main loop.
 * @details Manages system state transitions, timers, screen rendering, card reader
 *          decoding, and encoder interactions.
 */
void App_Run (void)
{
    uint8_t mode;
    bool private = (state == WAITING_PASSWORD) ? true : false; /* Obfuscate input for passwords */
    uint8_t length;
    uint8_t * data;

    /* =========================================================================
     * FINITE STATE MACHINE (FSM) - UI & TIMERS
     * ========================================================================= */
    switch(state)
    {
    /* State: Display initial ID prompt screen ("-Id-") */
    case ASKING_ID:
        length = 4;
        data = id_msg;
        status = 0;
        mode = COMPLETE;

        /* Display message for 2 seconds before accepting ID input */
        if(timer_finished())
        {
            state = WAITING_ID;
        } else
        {
            start_state_timer(2000);
        }
        break;

    /* States: User typing ID or system displaying verified ID */
    case WAITING_ID:
    case SHOWING_ID:
        length = id_counter;
        data = id;
        status = ONLY_FIRST_LED;
        mode = (state == SHOWING_ID) ? COMPLETE : EDITING;

        /* 20-second inactivity timeout: reset to ID prompt */
        if(timer_finished())
        {
            state = ASKING_ID;
            id_counter = 0;
            password_counter = 0;
        } else
        {
            start_state_timer(20000);
        }
        break;

    /* State: Error screen displayed when entered ID is not found ("IdnF") */
    case ID_NOT_FOUND:
        length = 4;
        data = id_nF;
        status = 0;
        mode = COMPLETE;

        /* Hold error screen for 2 seconds then return to ID entry */
        if(timer_finished())
        {
            state = WAITING_ID;
        } else
        {
            start_state_timer(2000);
        }
        break;

    /* State: Password entry prompt screen ("PSSd") */
    case ASKING_PASSWORD:
        length = 4;
        data = password_msg;
        status = FIRST_AND_SECOND_LED;
        mode = COMPLETE;

        /* Hold prompt for 2 seconds then enable password input */
        if(timer_finished())
        {
            state = WAITING_PASSWORD;
        } else
        {
            start_state_timer(2000);
        }
        break;

    /* States: User entering password or changing existing password */
    case WAITING_PASSWORD:
    case CHANGING_PASSWORD:
        length = password_counter;
        data = password;
        status = FIRST_AND_SECOND_LED;
        mode = EDITING;

        /* 20-second inactivity timeout: reset to initial state */
        if(timer_finished())
        {
            state = ASKING_ID;
            id_counter = 0;
            password_counter = 0;
        } else
        {
            start_state_timer(20000);
        }
        break;

    /* State: Access granted ("Good") - unlocking door mechanism */
    case OPENING:
        length = 4;
        data = good;
        status = ALL_LEDS_ON;
        mode = COMPLETE;

        /* Keep unlocked for 5 seconds, then reset system */
        if(timer_finished())
        {
            id_counter = 0;
            password_counter = 0;
            state = ASKING_ID;
        } else
        {
            start_state_timer(5000);
        }
        break;

    /* State: Access denied ("XXX") - invalid password entered */
    case WRONG_PASSWORD:
        length = password_tries;
        data = wrong;
        status = 0;
        mode = COMPLETE;

        /* Hold error for 1 second; evaluate remaining attempts */
        if(timer_finished())
        {
            if(password_tries < 3)
            {
                state = WAITING_PASSWORD; /* Allow retry */
            } else
            {
                state = ASKING_ID;        /* Lockout: reset to start */
                id_counter = 0;
            }
        } else
        {
            start_state_timer(1000);
        }
        break;

    /* State: Display brightness configuration mode */
    case BRIGHTNESS:
        length = (prev_state == WAITING_ID) ? id_counter : password_counter;
        data = (prev_state == WAITING_ID) ? id : password;
        status = FIRST_AND_THIRD_LED;
        mode = EDITING;
        private = (prev_state == WAITING_PASSWORD) ? true : false;
        setBrightness(brightness);
        break;

    /* State: System Administration Menu Sub-Tree */
    case ADMIN_MODE:
        switch(admin_sub_mode)
        {
        /* Menu option: View user count header ("Cant") */
        case ADMIN_SUB_CANT:
            length = 4;
            data = cant;
            status = SECOND_AND_THIRD_LED;
            mode = COMPLETE;
            break;

        /* Menu option: View user list header ("Id's") */
        case ADMIN_SUB_IDS:
            length = 4;
            data = ids;
            status = SECOND_AND_THIRD_LED;
            mode = COMPLETE;
            break;

        /* Menu option: Add new user header ("add") */
        case ADMIN_SUB_ADD:
            length = 3;
            data = add;
            status = SECOND_AND_THIRD_LED;
            mode = COMPLETE;
            break;

        /* Menu option: Delete user header ("dlt") */
        case ADMIN_SUB_DLT:
            length = 3;
            data = dlt;
            status = SECOND_AND_THIRD_LED;
            mode = COMPLETE;
            break;

        /* Menu option: Exit menu header ("EXIt") */
        case ADMIN_SUB_EXIT:
            length = 4;
            data = exit_msg;
            status = SECOND_AND_THIRD_LED;
            mode = COMPLETE;
            break;

        /* Sub-screen: Display current user quantity numerical value */
        case ADMIN_SUB_SHOW_CANT:
            length = 1;
            data = &users_cant;
            status = SECOND_AND_THIRD_LED;
            mode = COMPLETE;
            break;

        /* Sub-screens: Select user ID to view details or delete */
        case ADMIN_SUB_SELECT_ID:
        case ADMIN_SUB_DELETE_ID:
            length = 0;
            status = SECOND_AND_THIRD_LED;
            mode = EDITING;
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    /* Output updated state details to display hardware */
    print(data, length, selection, mode, private, row, status);

    /* =========================================================================
     * CARD READER PROCESSING
     * ========================================================================= */
    static track2_card_t card;
    if(state == WAITING_ID && data_ready())
    {
        /* Decode swiped card track 2 data */
        if(card_decode_track2(get_data(), get_data_length(), &card))
        {
            if(card.pan_length >= 8)
            {
                /* Extract first 8 PAN digits to form ID */
                for(int i = 0; i < 8; i++)
                {
                    id[i] = card.pan[i];
                    id_counter = 8;
                }

                /* Validate extracted ID against registered users */
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
     * ENCODER ROTATION HANDLING
     * ========================================================================= */
    if(encoderMoved())
    {
        if(state == WAITING_ID)
        {
            changeSelection(encoderDir(), id_counter == ID_LENGTH);
        } else if(state == SHOWING_ID)
        {
            row = (row + 1) & 0x01; /* Toggle row index */
        } else if(state == WAITING_PASSWORD || state == CHANGING_PASSWORD)
        {
            changeSelection(encoderDir(), password_counter == PASSWORD_MAX_LENGHT);
        } else if(state == BRIGHTNESS)
        {
            changeBrightness(encoderDir());
        } else if(state == ADMIN_MODE)
        {
            if(admin_sub_mode < ADMIN_SUB_SHOW_CANT)
            {
                adminMenu(encoderDir()); /* Scroll main admin menu options */
            } else if(admin_sub_mode == ADMIN_SUB_SELECT_ID || admin_sub_mode == ADMIN_SUB_DELETE_ID)
            {
                changeIdMenuAdmin(encoderDir()); /* Scroll user ID list in admin mode */
            }
        }
    }

    /* =========================================================================
     * ENCODER BUTTON PRESS HANDLING (Falling Edge Trigger)
     * ========================================================================= */
    static bool button_pressed_flag = 0;
    if(buttonPressed())
    {
        if(!button_pressed_flag)
        {
            button_pressed_flag = 1; /* Set edge flag */

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
                switch(admin_sub_mode)
                {
                /* Action 'Cant': Open screen displaying user quantity */
                case ADMIN_SUB_CANT:
                    admin_sub_mode = ADMIN_SUB_SHOW_CANT;
                    break;

                /* Action 'Id's': Enter ID list selection mode */
                case ADMIN_SUB_IDS:
                    admin_sub_mode = ADMIN_SUB_SELECT_ID;
                    selection = (users_cant != 0) ? 1 : E;
                    break;

                /* Action 'add': Switch to new user creation input sequence */
                case ADMIN_SUB_ADD:
                    adding_user = true;
                    id_counter = 0;
                    password_counter = 0;
                    selection = 0;
                    state = ASKING_ID;
                    break;

                /* Action 'dlt': Enter user deletion selection mode */
                case ADMIN_SUB_DLT:
                    admin_sub_mode = ADMIN_SUB_DELETE_ID;
                    selection = (users_cant != 0) ? 1 : E;
                    break;

                /* Action 'EXIt': Exit admin mode back to main ID screen */
                case ADMIN_SUB_EXIT:
                    id_counter = 0;
                    password_counter = 0;
                    selection = 0;
                    state = ASKING_ID;
                    break;

                /* Action in quantity screen: Return to top-level admin menu */
                case ADMIN_SUB_SHOW_CANT:
                    admin_sub_mode = ADMIN_SUB_CANT;
                    break;

                /* Action in ID list: Show full ID or exit if 'E' was selected */
                case ADMIN_SUB_SELECT_ID:
                    if(selection == E)
                    {
                        admin_sub_mode = ADMIN_SUB_IDS;
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
                    break;

                /* Action in deletion list: Remove selected user and shift array */
                case ADMIN_SUB_DELETE_ID:
                    if(selection == E)
                    {
                        admin_sub_mode = ADMIN_SUB_DLT;
                    } else
                    {
                        for(int i = selection; i < users_cant; i++)
                        {
                            users[i] = users[i+1];
                        }
                        users_cant--;
                        admin_sub_mode = ADMIN_SUB_DLT;
                    }
                    break;

                default:
                    break;
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

/**
 * @brief Helper function to start state timeouts cleanly without code duplication.
 * @param ms Delay duration in milliseconds.
 */
static void start_state_timer(uint32_t ms)
{
    if(!timer_counting() || first_in_state == true)
    {
        start_timer_ms(ms);
        first_in_state = false;
    }
}


/**
 * @brief Updates the menu selection index based on rotary encoder direction and state.
 * @details Adjusts `selection` boundaries depending on whether the current input buffer
 *          is complete or if minimum password length constraints are met.
 * @param dir Direction of rotation (IS_RIGHT for clockwise, otherwise counter-clockwise).
 * @param complete Flag indicating if the active input field (ID/Password) has reached max length.
 */
void changeSelection(bool dir, bool complete)
{
    uint8_t max = SELECTION_MODES - 2;
    uint8_t min = 0;

    /* Expand selection bounds if input buffer is full */
    if(complete)
    {
        max = SELECTION_MODES - 1;
        min = 10;
    }

    /* Expand upper boundary if password meets minimum length requirement */
    if(password_counter >= PASSWORD_MIN_LENGHT)
    {
        max = SELECTION_MODES - 1;
    }

    /* Navigate selection index with rollover limits */
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

/**
 * @brief Navigates the registered user ID selection list within the admin menu.
 * @details Cycles through valid user indices (1 to `users_cant`) and the exit option ('E').
 * @param dir Direction of movement (IS_RIGHT for next option, otherwise previous option).
 */
void changeIdMenuAdmin(bool dir)
{
    uint8_t min = 1;
    uint8_t max;

    /* Set boundary parameters according to current user count */
    if(users_cant == 0)
    {
        min = E;
        max = E;
    } else
    {
        min = E;
        max = users_cant;
    }

    /* Cycle forward or backward through user list and exit option */
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

/**
 * @brief Handles user input confirmation actions based on the active state and selection value.
 * @details Processes numeric digit additions (0-9), control actions like backspace (10),
 *          clear buffer (11), brightness setup (12), password change triggers (13),
 *          cancellation (14), and submission/verification commands (15).
 */
void selectionEntered(void)
{
    uint8_t * counter;
    uint8_t * data;
    uint8_t max;

    /* Bind active buffer pointers based on current state */
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

    /* Selection 0-9: Numeric digit input */
    if(selection >= 0 && selection <= 9)
    {
        if(*counter < max)
        {
            data[(*counter)++] = selection;
        }
    } 
    /* Selection 10: Backspace (delete last character) */
    else if(selection == KEY_BACKSPACE)
    {
        if((*counter) != 0)
        {
            (*counter)--;
        }
    } 
    /* Selection 11: Clear whole input buffer */
    else if(selection == KEY_CLEAR)
    {
        (*counter) = 0;
    } 
    /* Selection 12: Open brightness configuration screen */
    else if(selection == KEY_BRIGHTNESS)
    {
        prev_state = state;
        state = BRIGHTNESS;
        first_in_state = true;
    }
    /* Selection 13: Initiate password change sequence */
    else if(selection == KEY_CHANGE_PASS)
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
    }
    /* Selection 14: Cancel input and return to ID prompt */
    else if(selection == KEY_CANCEL)
    {
        id_counter = 0;
        password_counter = 0;
        state = WAITING_ID;
        first_in_state = true;
    } 
    /* Selection 15: Enter / Confirm input submission */
    else if(selection == KEY_ENTER)
    {
        if(state == WAITING_ID)
        {
            /* Validate entered ID or accept if admin is adding a new user */
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
            /* Verify password or process admin new user insertion */
            if(matchPassword() || adding_user)
            {
                if(active_user == 0 && !adding_user)
                {
                    /* Admin user logged in */
                    state = ADMIN_MODE;
                    selection = 0;
                    first_in_state = true;
                } else if(adding_user)
                {
                    /* Commit new user credentials to database */
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
                    admin_sub_mode = ADMIN_SUB_CANT;
                } else
                {
                    /* Standard user access granted */
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
            /* Overwrite current active user password */
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

    /* Reset default UI selection focus based on input buffer state */
    if(state != BRIGHTNESS)
    {
        if((*counter) < max)
        {
            selection = 0;
        } else
        {
            selection = KEY_ENTER;
        }
    }
}

/**
 * @brief Checks if the entered ID exists in the user database.
 * @details Compares the active `id` buffer with stored user IDs. If matched,
 *          updates `active_user` with the corresponding user array index.
 * @return true if ID matches a registered user, false otherwise.
 */
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

/**
 * @brief Validates if the input password matches the active user's stored password.
 * @return true if password matches completely, false otherwise.
 */
bool matchPassword(void)
{
    uint8_t m = 0;
    while((password[m] == users[active_user].password[m]) && (m < users[active_user].password_length))
    {
        m++;
    }

    return m == users[active_user].password_length;
}

/**
 * @brief Adjusts display brightness within configured boundaries.
 * @param dir Direction to shift brightness (IS_RIGHT increases, otherwise decreases).
 */
void changeBrightness(bool dir)
{
    if((dir == IS_RIGHT) && (brightness < HUNDRED_PERCENT_BRIGTHNESS))
    {
        brightness++;
    } else if((dir != IS_RIGHT) && (brightness > TEN_PERCENT_BRIGTHNESS))
    {
        brightness--;
    }
}

/**
 * @brief Cycles through top-level options in the admin menu state.
 * @param dir Direction of movement (IS_RIGHT moves to next sub-mode, otherwise previous).
 */
void adminMenu(bool dir)
{
    if(dir == IS_RIGHT)
    {
        if(admin_sub_mode == ADMIN_SUB_EXIT)
        {
            admin_sub_mode = ADMIN_SUB_CANT;
        } else
        {
            admin_sub_mode++;
        }
    } else
    {
        if(admin_sub_mode == ADMIN_SUB_CANT)
        {
            admin_sub_mode = ADMIN_SUB_EXIT;
        } else
        {
            admin_sub_mode--;
        }
    }
}
