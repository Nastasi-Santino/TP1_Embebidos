#ifndef SERIAL_OUT_H_
#define SERIAL_OUT_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SER_CLK_PERIOD_US   10U  /**< Serial software clock toggle period in microseconds */

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes GPIO output pins and PISR periodic clock timer for serial transmission.
 * @return True if pin modes and clock timer initialized successfully, false otherwise.
 */
bool serial_out_INIT(void);

/**
 * @brief Serializes segment, column selection, and LED status data into shift registers.
 * @param seg 7-segment digit bitmask payload byte.
 * @param sel Target active display column/digit selection index mask.
 * @param status Indicator LED output combination mask.
 */
void serial_out(uint8_t seg, uint8_t sel, uint8_t status);

#endif /* SERIAL_OUT_H_ */