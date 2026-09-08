/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "card_reader.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PIN_CR_ENABLE   PORTNUM2PIN(PD, 1) /**< Card reader Enable/Card-Present line (PTD1) */
#define PIN_CR_CLOCK    PORTNUM2PIN(PD, 3) /**< Card reader Clock input line (PTD3) */
#define PIN_CR_DATA     PORTNUM2PIN(PD, 2) /**< Card reader Active-Low Data input line (PTD2) */

#define BUFFER_MAX_BITS 250                /**< Maximum capacity of incoming bit array buffer */

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Interrupt Handler triggered by state changes on Card Enable pin.
 */
void enable_IRQHandler(void);

/**
 * @brief Interrupt Handler triggered on falling edges of Card Clock signal.
 */
void clock_IRQHandler(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static volatile uint8_t dataBuffer[BUFFER_MAX_BITS] = {0}; /**< Storage array for raw sampled bits */
static volatile uint8_t bitCount;                           /**< Total bit count accumulated during swipe */
static volatile bool reading;                               /**< Flag indicating active card pass in progress */
static volatile bool dataReady;                             /**< Flag set when a card swipe completes successfully */

/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Configures GPIO inputs and interrupt routines for the card reader.
 * @return True if pin modes and interrupts were attached successfully, false otherwise.
 */
bool card_reader_INIT(void)
{
    /* Set required signal pins as inputs */
    gpioMode(PIN_CR_ENABLE, INPUT);
    gpioMode(PIN_CR_CLOCK, INPUT);
    gpioMode(PIN_CR_DATA, INPUT);

    /* Attach interrupt routines for signal lines */
    bool enable_flag = gpioIRQ(PIN_CR_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, enable_IRQHandler);
    bool clock_flag  = gpioIRQ(PIN_CR_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, clock_IRQHandler);

    /* Reset driver flags and state variables */
    bitCount = 0;
    reading = false;
    dataReady = false;

    if(!enable_flag || !clock_flag)
    {
        return false;
    }

    return true;
}

/**
 * @brief Checks if new card data has been fully captured and is pending processing.
 * @details Automatically clears the ready status flag upon query.
 * @return True if fresh card data is ready for reading, false otherwise.
 */
bool data_ready(void)
{
    if(dataReady)
    {
        dataReady = false; /* Clear flag upon reading */
        return true;
    }

    return false;
}

/**
 * @brief Retrieves the total number of bits collected during the last swipe.
 * @return Number of recorded bits in the payload buffer.
 */
uint8_t get_data_length(void)
{
    return bitCount;
}

/**
 * @brief Provides access to the raw sampled bit buffer.
 * @return Read-only pointer to volatile buffer containing captured bits.
 */
const volatile uint8_t * get_data(void)
{
    return dataBuffer;
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINES & LOCAL FUNCTIONS
 ******************************************************************************/

/**
 * @brief ISR managing card insertion/exit detection via Enable line edge changes.
 * @details Active Low line: High signal indicates card left reader (end of transmission),
 *          Low signal indicates active card pass sequence starting.
 */
void enable_IRQHandler(void)
{
    if(gpioRead(PIN_CR_ENABLE))
    {
        /* Enable line went HIGH: Card swipe completed */
        reading = false;
        dataReady = true;
    } else
    {
        /* Enable line went LOW: Card swipe started, clear buffer counters */
        reading = true;
        dataReady = false;
        bitCount = 0;
    }
}

/**
 * @brief ISR capturing data bits on each falling edge of the Clock signal.
 * @details Reads active-low Data pin state, inverts it to standard logic,
 *          and appends bit value into the buffer.
 */
void clock_IRQHandler(void)
{
    if(reading && (bitCount < BUFFER_MAX_BITS))
    {
        /* Sample Active-Low data line (inverted) and increment buffer pointer */
        dataBuffer[bitCount] = !gpioRead(PIN_CR_DATA);
        bitCount++;
    }
}
