#include "encoder.h"
#include "pisr.h"

#define BUTTON_PERIOD_MS        50U
#define ENCODER_PERIOD_MS		25U
#define ENCODER_COUNTER			5

#define PISR_MS_TO_TICKS(ms) \
    (((ms) * 1000U) / PISR_TICK_US)

#define BUTTON_PERIOD_TICKS     PISR_MS_TO_TICKS(BUTTON_PERIOD_MS)
#define ENCODER_PERIOD_TICKS	PISR_MS_TO_TICKS(ENCODER_PERIOD_MS/ENCODER_COUNTER)

static void button_PISR(void);
static void A_IRQHandler(void);
static void B_IRQHandler(void);
static void encoder_PISR(void);

static pin_t button;
static pin_t a;
static pin_t b;
static bool counting_a;
static bool counting_b;
static bool a_active;
static bool b_active;

static bool buttonPressed_flag;
static bool	encoderMoved_flag;
static bool encoderDir_flag;

bool encoder_INIT(pin_t buttonPin, pin_t aPin, pin_t bPin)
{
	gpioMode(buttonPin, INPUT);
	gpioMode(aPin, INPUT);
	gpioMode(bPin, INPUT);

	button = buttonPin;
	a = aPin;
	b = bPin;

	bool a_flag = gpioIRQ(aPin, GPIO_IRQ_MODE_RISING_EDGE, A_IRQHandler);
	bool b_flag = gpioIRQ(bPin, GPIO_IRQ_MODE_RISING_EDGE, B_IRQHandler);

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
	return encoderMoved_flag;
}

bool encoderDir(void)
{
	return encoderDir_flag;
}

static void button_PISR(void)
{
	if(gpioRead(button))
	{
		buttonPressed_flag = 1;
	} else
	{
		buttonPressed_flag = 0;
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
			if(gpioRead(a))
			{
				a_active = true;
				if(b_active)
				{
					encoderMoved_flag = true;
					encoderDir_flag = IS_LEFT;
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
			if(gpioRead(b))
			{
				b_active = true;
				if(a_active)
				{
					encoderMoved_flag = true;
					encoderDir_flag = IS_RIGHT;
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
