#ifndef _DELAY_H
#define _DELAY_H

__IO uint32_t msTick;

void SysTick_Handler(void);
void delay_init(unsigned int freq);
void delay(unsigned int tick);

#endif

