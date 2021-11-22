#include "stm32f4xx.h"
#include "delay.h"

void delay_init(unsigned int freq)
{
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / freq);
}
void delay(unsigned int tick)
{
	msTick = tick;
	while (msTick);
}

