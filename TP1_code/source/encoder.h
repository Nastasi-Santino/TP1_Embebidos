#ifndef ENCODER_H_
#define ENCODER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

enum {
	IS_LEFT,
	IS_RIGHT
};

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

bool encoder_INIT(pin_t buttonPin, pin_t aPin, pin_t bPin);

bool buttonPressed(void);

bool encoderMoved(void);

bool encoderDir(void);


#endif /* ENCODER_H_ */
