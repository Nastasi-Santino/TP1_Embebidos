#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DISPLAY_COUNT	 	4U
#define REFRESH_RATE_HZ		100U

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

#endif /* DISPLAY_H_ */
