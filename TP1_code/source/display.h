#ifndef DISPLAY_H_
#define DISPLAY_H_


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DISPLAY_COUNT	 	4U
#define REFRESH_RATE_HZ		100U
#define ROW_PERIOD_MS		2000U

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum
{
	EDITING,
	COMPLETE,
};


typedef struct
{
	uint8_t seg;
	uint8_t column;
}display_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

bool display_INIT(void);
display_t print(uint8_t * data, uint8_t data_length, uint8_t selection, uint8_t mode, bool private);

#endif /* DISPLAY_H_ */
