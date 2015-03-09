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
#include <lcd.h>
#include <lcd_lld.h>
#include <color.h>
#include <stm32f4xx.h>

GPIO_InitTypeDef gi;



int main(void)
{
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC,ENABLE);
	RCC->AHB1ENR|= RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC;

	//Led---------------------------------

	//GPIO_StructInit(&gi);
	//gi.GPIO_Pin = GPIO_Pin_0;

	//gi.GPIO_Mode = GPIO_Mode_OUT;
	GPIOA->MODER &=~ 3;
	GPIOA->MODER |= GPIO_Mode_OUT;

	//gi.GPIO_OType = GPIO_OType_PP;
	GPIOA->OTYPER &=~ 1;
	GPIOA->OTYPER |= GPIO_OType_PP;


	//gi.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIOA->PUPDR &=~ 3;
	GPIOA->PUPDR |= GPIO_PuPd_NOPULL;

	//GPIO_Init(GPIOA,&gi);


	//Taster---------------------------

	//GPIO_StructInit(&gi);
	//gi.GPIO_Pin = GPIO_Pin_7;


	//gi.GPIO_Mode = GPIO_Mode_IN;
	GPIOC->MODER &=~ ( 3 << (7*2));
	GPIOC->MODER |= GPIO_Mode_IN << (7*2);


	//gi.GPIO_OType = GPIO_OType_OD;
	GPIOC->OTYPER &=~ (1 << (7*2));
	GPIOC->OTYPER |= GPIO_OType_OD <<(7*2);

	//gi.GPIO_PuPd = GPIO_PuPd_UP;
	GPIOC->PUPDR &=~  (3 <<(7*2));
	GPIOC->PUPDR |= GPIO_PuPd_UP <<(7*2);


	//GPIO_Init(GPIOC,&gi);

	while(1) {
		//if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)) {
		if(GPIOC->IDR & GPIO_Pin_7) {

			//GPIO_SetBits(GPIOA,GPIO_Pin_0);
			GPIOA->ODR |=GPIO_Pin_0;


		} else {
			//GPIO_ResetBits(GPIOA,GPIO_Pin_0);
			GPIOA->ODR &=~GPIO_Pin_0;

		}

	}


	return 0;
}

