#include "encoder.h"
#include "pisr.h"

#define PIN_SW_ENCODER	PORTNUM2PIN(PC, 11)
#define PIN_A_ENCODER	PORTNUM2PIN(PB, 18)
#define PIN_B_ENCODER	PORTNUM2PIN(PC, 10)

#define BUTTON_PERIOD_MS        50U
#define ENCODER_PERIOD_MS		3U
#define ENCODER_COUNTER			3

#define PISR_MS_TO_TICKS(ms) \
    (((ms) * 1000U) / PISR_TICK_US)

#define BUTTON_PERIOD_TICKS     PISR_MS_TO_TICKS(BUTTON_PERIOD_MS)
#define ENCODER_PERIOD_TICKS	PISR_MS_TO_TICKS(ENCODER_PERIOD_MS/ENCODER_COUNTER)

static void button_PISR(void);
static void A_IRQHandler(void);
static void B_IRQHandler(void);
static void encoder_PISR(void);

static bool counting_a;
static bool counting_b;
static bool a_active;
static bool b_active;

static bool buttonPressed_flag;
static bool	encoderMoved_flag;
static bool encoderDir_flag;

bool encoder_INIT(void)
{
	gpioMode(PIN_SW_ENCODER, INPUT);
	gpioMode(PIN_A_ENCODER, INPUT);
	gpioMode(PIN_B_ENCODER, INPUT);

	bool a_flag = gpioIRQ(PIN_A_ENCODER, GPIO_IRQ_MODE_FALLING_EDGE, A_IRQHandler);
	bool b_flag = gpioIRQ(PIN_B_ENCODER, GPIO_IRQ_MODE_FALLING_EDGE, B_IRQHandler);

	bool button_flag = pisrRegister(button_PISR, BUTTON_PERIOD_TICKS);
	bool encoder_flag =	pisrRegister(encoder_PISR, ENCODER_PERIOD_TICKS);

	if(!a_flag || !b_flag || !button_flag || !encoder_flag)
	{
		return false;
	}

	return true;
}

bool buttonPressed(void)
{
	return buttonPressed_flag;
}

bool encoderMoved(void)
{
	if(encoderMoved_flag)
	{
		encoderMoved_flag = false;
		return true;
	}

	return false;
}

bool encoderDir(void)
{
	return encoderDir_flag;
}

static void button_PISR(void)
{
	if(gpioRead(PIN_SW_ENCODER))
	{
		buttonPressed_flag = 0;
	} else
	{
		buttonPressed_flag = 1;
	}
}

static void A_IRQHandler(void)
{
	if(!counting_a)
	{
		counting_a = true;
	}
}


static void B_IRQHandler(void)
{
	if(!counting_b)
	{
		counting_b = true;
	}
}

static void encoder_PISR(void)
{
	static uint8_t counter_a = 0;
	static uint8_t counter_b = 0;

	if(counting_a)
	{
		if(counter_a >= ENCODER_COUNTER)
		{
			if(!gpioRead(PIN_A_ENCODER))
			{
				a_active = true;
				if(b_active)
				{
					encoderMoved_flag = true;
					encoderDir_flag = IS_RIGHT;
					a_active = false;
					b_active = false;
				}
			} else
			{
				a_active = false;
			}
			counter_a = 0;
			counting_a = false;
		}
		counter_a++;
	}

	if(counting_b)
	{
		if(counter_b >= ENCODER_COUNTER)
		{
			if(!gpioRead(PIN_B_ENCODER))
			{
				b_active = true;
				if(a_active)
				{
					encoderMoved_flag = true;
					encoderDir_flag = IS_LEFT;
					a_active = false;
					b_active = false;
				}
			} else
			{
				b_active = false;
			}
			counter_b = 0;
			counting_b = false;
		}
		counter_b++;
	}
}
