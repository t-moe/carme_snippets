/*****************************************************************************
 * Copyright &copy; 2013, Bern University of Applied Sciences.
 * All rights reserved.
 *
 * ##### GNU GENERAL PUBLIC LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 *****************************************************************************/
#include <stm32f4xx.h>				// Processor STM32F407IG
#include <carme.h>					// CARME Module

volatile unsigned char* LED = (volatile unsigned char*)0x6C000200;
volatile unsigned char* SWITCH = (volatile unsigned char*)0x6C000400;

void TIM6_DAC_IRQHandler(void) {
	if(TIM_GetFlagStatus(TIM6,TIM_FLAG_Update)) {
	    *LED^=1;
		TIM_ClearFlag(TIM6,TIM_FLAG_Update);
	}
}

void TIM7_IRQHandler(void) {
	if(TIM_GetFlagStatus(TIM7,TIM_FLAG_Update)) {
	    *LED^=2;
		TIM_ClearFlag(TIM7,TIM_FLAG_Update);
	}
}


int main(void)
{
	const int APB1_CLK=42E6; //42mhz APB1 Takt
	TIM_TimeBaseInitTypeDef tinit;

	*LED = 0;

	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM6EN,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM7EN,ENABLE);
	__disable_irq();

	TIM_TimeBaseStructInit(&tinit);
	tinit.TIM_Prescaler = APB1_CLK/1000 -1; //0...41999
	tinit.TIM_Period = 1000-1; //counts from 0 to 999
	TIM_TimeBaseInit(TIM6,&tinit);
	tinit.TIM_Period = 250-1;
	TIM_TimeBaseInit(TIM7,&tinit);

	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	NVIC_EnableIRQ(TIM7_IRQn);

	__enable_irq();
	TIM_Cmd(TIM6,ENABLE);
	TIM_Cmd(TIM7,ENABLE);

	while(1);

	return 0;
}

