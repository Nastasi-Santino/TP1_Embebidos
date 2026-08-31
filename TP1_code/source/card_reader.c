#include "card_reader.h"

#define PIN_CR_ENABLE	PORTNUM2PIN(PD, 1)
#define PIN_CR_CLOCK	PORTNUM2PIN(PD, 3)
#define PIN_CR_DATA		PORTNUM2PIN(PD, 2)

static volatile uint8_t dataBuffer[250] = {0};
static volatile uint8_t bitCount;
static volatile bool reading;
static volatile bool dataReady;

void enable_IRQHandler(void);
void clock_IRQHandler(void);

bool card_reader_INIT(void)
{
	gpioMode(PIN_CR_ENABLE, INPUT);
	gpioMode(PIN_CR_CLOCK, INPUT);
	gpioMode(PIN_CR_DATA, INPUT);

	bool enable_flag = gpioIRQ(PIN_CR_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES, enable_IRQHandler);
	bool clock_flag = gpioIRQ(PIN_CR_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, clock_IRQHandler);

	bitCount = 0;
	reading = false;
	dataReady = false;

	if(!enable_flag || !clock_flag)
	{
		return false;
	}

	return true;
}

bool data_ready(void)
{
	if(dataReady)
	{
		dataReady = false;
		return true;
	}

	return false;
}

uint8_t get_data_length(void)
{
	return bitCount;
}

const volatile uint8_t * get_data(void)
{
	return dataBuffer;
}

void enable_IRQHandler(void)
{
	if(gpioRead(PIN_CR_ENABLE))
	{
		reading = false;
		dataReady = true;
	} else
	{
		reading = true;
		dataReady = false;
		bitCount = 0;
	}

}

void clock_IRQHandler(void)
{
	if(reading && (bitCount < 250))
	{
		dataBuffer[bitCount] = !gpioRead(PIN_CR_DATA);
		bitCount++;
	}
}
