#include "card_reader.h"

static volatile uint8_t dataBuffer[250] = {0};
static volatile uint8_t bitCount;
static volatile bool reading;
static volatile bool dataReady;

static pin_t data;
static pin_t enable;

void enable_IRQHandler(void);
void clock_IRQHandler(void);

bool card_reader_INIT(pin_t enablePin, pin_t clockPin, pin_t dataPin)
{
	gpioMode(enablePin, INPUT);
	gpioMode(clockPin, INPUT);
	gpioMode(dataPin, INPUT);

	data = dataPin;
	enable = enablePin;

	bool enable_flag = gpioIRQ(enablePin, GPIO_IRQ_MODE_BOTH_EDGES, enable_IRQHandler);
	bool clock_flag = gpioIRQ(clockPin, GPIO_IRQ_MODE_FALLING_EDGE, clock_IRQHandler);

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
	if(gpioRead(enable))
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
		dataBuffer[bitCount] = gpioRead(data);
		bitCount++;
	}
}
