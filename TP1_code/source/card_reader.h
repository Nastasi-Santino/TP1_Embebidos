#ifndef CARD_READER_H_
#define CARD_READER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

bool card_reader_INIT(pin_t enablePin, pin_t clockPin, pin_t dataPin);

bool data_ready(void);

uint8_t get_data_length(void);
const volatile uint8_t * get_data(void);



#endif /* CARD_READER_H_ */
