#ifndef SERIAL_OUT_H_
#define SERIAL_OUT_H_

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SER_CLK_PERIOD_US	100U

bool serial_out_INIT(void);

void serial_out(uint8_t seg, uint8_t sel, uint8_t status);

#endif /* SERIAL_OUT_H_ */
