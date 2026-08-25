#include "gpio.h"
#include "hardware.h"

static GPIO_Type * const gpios[] = GPIO_BASE_PTRS;

static PORT_Type * const ports[] = PORT_BASE_PTRS;

static uint32_t portsIRQn[] = {PORTA_IRQn,
		PORTB_IRQn, PORTC_IRQn,
		PORTD_IRQn, PORTE_IRQn};


void gpioMode (pin_t pin, uint8_t mode)
{
	GPIO_Type * gpio = gpios[PIN2PORT(pin)];

	PORT_Type * port = ports[PIN2PORT(pin)];

	port->PCR[PIN2NUM(pin)] &= ~(7 << PORT_PCR_MUX_SHIFT);
	port->PCR[PIN2NUM(pin)] |= (1 << PORT_PCR_MUX_SHIFT);
	port->PCR[PIN2NUM(pin)] &= ~PORT_PCR_PE_MASK;

	if(mode == OUTPUT)
	{
		gpio->PDDR |= (uint32_t)((uint32_t)1 << PIN2NUM(pin));
	} else if (mode == INPUT){
		gpio->PDDR &= ~((uint32_t)1  << PIN2NUM(pin));
	} else{
		port->PCR[PIN2NUM(pin)] |= PORT_PCR_PE_MASK |
				((uint32_t)((uint8_t)INPUT_PULLDOWN - mode)
				<< PORT_PCR_PS_SHIFT); // Activo PE y PS
	}
}

static pinIrqFun_t portHandler[5][32];
static int porIrqCounters[5];
static int activeHandlers[5][32];

bool gpioIRQ(pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun)
{
	if(irqFun == (pinIrqFun_t)0)
		return 0;

	if(irqMode > 3)
		return 0;

	PORT_Type * port = ports[PIN2PORT(pin)];

	port->PCR[PIN2NUM(pin)] &= ~(15U << PORT_PCR_IRQC_SHIFT);

	port->ISFR = (1U << PIN2NUM(pin));

	port->PCR[PIN2NUM(pin)] |= (irqMode ? irqMode + 8U : 0) << PORT_PCR_IRQC_SHIFT;

	portHandler[PIN2PORT(pin)][PIN2NUM(pin)] = irqFun;

	activeHandlers[PIN2PORT(pin)][porIrqCounters[PIN2PORT(pin)]++] = PIN2NUM(pin);

	NVIC_EnableIRQ(portsIRQn[PIN2PORT(pin)]);

	return 1;
}

void gpioWrite(pin_t pin, bool value)
{
	GPIO_Type * gpio = gpios[PIN2PORT(pin)];

	if(value){
		gpio->PSOR = ((uint32_t)1 << PIN2NUM(pin));
	} else
		gpio->PCOR = ((uint32_t)1 << PIN2NUM(pin));
}

void gpioToggle(pin_t pin)
{
	GPIO_Type * gpio = gpios[PIN2PORT(pin)];

	gpio->PTOR = ((uint32_t)1 << PIN2NUM(pin));
}

bool gpioRead(pin_t pin)
{
	GPIO_Type * gpio = gpios[PIN2PORT(pin)];

	return ((uint32_t)1 << PIN2NUM(pin)) ==
			(gpio->PDIR & ((uint32_t)1 << PIN2NUM(pin)));
}

bool gpioDigitalFilter(pin_t pin, bool clk, uint32_t count)
{
	if(count > 31)
	{
		return 0;
	}

	PORT_Type * port = ports[PIN2PORT(pin)];

	port->DFWR &= ~(31 << 0U);
	port->DFWR |= (count << 0U);

	port->DFER |= (1 << PIN2NUM(pin));

	if(clk)
	{
		port->DFCR &= (1 << 0U);
	} else
	{
		port->DFCR |= (1 << 0U);
	}

	return 1;
}

static void gpioPortIRQHandler(uint8_t portNum)
{
	for(int i = 0; i < porIrqCounters[portNum]; i++)
	{
		if((ports[portNum]->ISFR & (1U << activeHandlers[portNum][i])) != 0U)
		{
			ports[portNum]->ISFR = 1U << activeHandlers[portNum][i];
			portHandler[portNum][activeHandlers[portNum][i]]();
		}
	}
}

void PORTA_IRQHandler(void)
{
	gpioPortIRQHandler(PA);
}

void PORTB_IRQHandler(void)
{
	gpioPortIRQHandler(PB);
}

void PORTC_IRQHandler(void)
{
	gpioPortIRQHandler(PC);
}

void PORTD_IRQHandler(void)
{
	gpioPortIRQHandler(PD);
}

void PORTE_IRQHandler(void)
{
	gpioPortIRQHandler(PE);
}
