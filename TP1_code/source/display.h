#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DISPLAY_COUNT	 	4U
#define REFRESH_RATE_HZ		100U

enum{
	ALL_LEDS_OFF,
	ONLY_FIRST_LED,
	ONLY_SECOND_LED,
	ONLY_THIRD_LED,
	FIRST_AND_SECOND_LED,
	SECOND_AND_THIRD_LED,
	FIRST_AND_THIRD_LED,
	ALL_LEDS_ON
};

enum{
	TEN_PERCENT_BRIGTHNESS = 1,
	TWENTY_PERCENT_BRIGTHNESS,
	THIRTY_PERCENT_BRIGTHNESS,
	FORTY_PERCENT_BRIGTHNESS,
	FIFTY_PERCENT_BRIGTHNESS,
	SIXTY_PERCENT_BRIGTHNESS,
	SEVENTY_PERCENT_BRIGTHNESS,
	EIGHTY_PERCENT_BRIGTHNESS,
	NINETY_PERCENT_BRIGTHNESS,
	HUNDRED_PERCENT_BRIGTHNESS
};

/*******************************************************************************
 * LETTERS
 ******************************************************************************/

#define G 6
#define o 17
#define d 18
#define X 11
#define GUION 16
#define I 19
#define n 20
#define F 21
#define P 22
#define S 5
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum
{
	EDITING,
	COMPLETE,
};


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

bool display_INIT(void);

void print(uint8_t * data, uint8_t data_length,
		uint8_t selection, uint8_t mode, bool private, uint8_t row, uint8_t status);

void setBrightness(uint8_t brightness);

#endif /* DISPLAY_H_ */
