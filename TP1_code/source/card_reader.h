#ifndef CARD_READER_H_
#define CARD_READER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"  /**< General Purpose Input/Output driver interface */

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes GPIO pins and interrupt handlers for the magnetic card reader.
 * @return True if initialization and interrupt setup succeeded, false otherwise.
 */
bool card_reader_INIT(void);

/**
 * @brief Checks if a full card swipe dataset has been captured and is ready for reading.
 * @details Automatically clears the ready status flag upon execution.
 * @return True if new card data is pending processing, false otherwise.
 */
bool data_ready(void);

/**
 * @brief Retrieves the total count of bits captured during the last card swipe.
 * @return Number of recorded bits in the data buffer.
 */
uint8_t get_data_length(void);

/**
 * @brief Provides a read-only pointer to the raw captured card bit buffer.
 * @return Pointer to volatile array containing captured bit state values.
 */
const volatile uint8_t * get_data(void);

#endif /* CARD_READER_H_ */