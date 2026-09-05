#include "serial_out.h"
#include "gpio.h"
#include "pisr.h"

#define PIN_SERIAL		PORTNUM2PIN(PB, 11)
#define PIN_SCLK		PORTNUM2PIN(PB, 2)
#define PIN_RCLK		PORTNUM2PIN(PB, 3)
#define PIN_OE			PORTNUM2PIN(PB, 10)

#define SCLK_TICKS PISR_US_TO_TICKS(SER_CLK_PERIOD_US)

void serCLK_geneator(void);

static bool serCLK;


bool serial_out_INIT(void)
{
	gpioMode(PIN_SERIAL, OUTPUT);
	gpioMode(PIN_SCLK, OUTPUT);
	gpioMode(PIN_RCLK, OUTPUT);
	gpioMode(PIN_OE, OUTPUT);

	gpioWrite(PIN_SERIAL, LOW);
	gpioWrite(PIN_SCLK, LOW);
	gpioWrite(PIN_RCLK, LOW);
	gpioWrite(PIN_OE, LOW);


	if(!pisrRegister(serCLK_geneator, SCLK_TICKS))
	{
		return false;
	}
	return true;
}

void serial_out(uint8_t seg, uint8_t sel, uint8_t status)
{
	bool data;
	uint8_t counter = 0;
	bool first = true;
	bool flag = true;
	while(counter < 14 || !flag)
	{
		if(!serCLK)
		{
			first = false;
			if(flag)
			{
				if(counter < 2)
				{
					data = (sel & 0x02) == 0x02;
					sel <<= 1;
				} else if(counter < 4)
				{
					data = (status & 0x02) == 0x02;
					status <<= 1;
				} else if(counter != 5 && counter != 13)
				{
					data = (seg & 0x01) == 0x01;
					seg >>= 1;
				} else{
					data = 0;
				}
				gpioWrite(PIN_SERIAL, data);
				gpioWrite(PIN_SCLK, 0);
				counter++;
				flag = false;
			}
		}  else
		{
			if(!first && !flag)
			{
				gpioWrite(PIN_SCLK, 1);
				flag = true;
			}
		}

	}

	gpioWrite(PIN_OE, HIGH);
	gpioWrite(PIN_RCLK, HIGH);
	flag = true;

	while(counter < 17)
	{
		if(serCLK)
		{
			if(flag)
			{
				counter++;
				flag = false;
			}
		} else
		{
			flag = true;
		}
	}


	gpioWrite(PIN_OE, LOW);
	gpioWrite(PIN_SCLK, 0);
	gpioWrite(PIN_RCLK, LOW);
}

void serCLK_geneator(void)
{
	serCLK = !serCLK;
}
