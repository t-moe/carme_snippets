#include <stm32f4xx.h>				// Processor STM32F407IG
#include <carme.h>					// CARME Module
#include <stm32f4xx.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct pin_s {
	GPIO_TypeDef* GPIO;
	uint16_t pinmask;
	bool input;
} pin_t;

pin_t* create_pin(GPIO_TypeDef* GPIO, uint8_t pinnr, bool input) {
	GPIO_InitTypeDef gi;
	GPIO_StructInit(&gi);

	gi.GPIO_Pin = 1 << pinnr;

	if(input) {
		gi.GPIO_Mode = GPIO_Mode_IN;
		gi.GPIO_OType = GPIO_OType_OD;
		gi.GPIO_PuPd = GPIO_PuPd_UP;
	} else {
		gi.GPIO_Mode = GPIO_Mode_OUT;
		gi.GPIO_OType = GPIO_OType_PP;
		gi.GPIO_PuPd = GPIO_PuPd_NOPULL;
	}

	GPIO_Init(GPIO,&gi);

	pin_t* s = malloc(sizeof(pin_t));
	s->GPIO=GPIO;
	s->pinmask=0x01<<pinnr;
	s->input = input;
	return s;
}

void destory_pin(pin_t* pin) {
	free(pin);
}

bool get_pin(pin_t* pin) {
	if(pin->input) {
		return GPIO_ReadInputDataBit(pin->GPIO,pin->pinmask);
	} else {
		return GPIO_ReadOutputDataBit(pin->GPIO,pin->pinmask);
	}
}

void set_pin(pin_t* pin, bool status) {
	if(!pin->input) {
		GPIO_WriteBit(pin->GPIO,pin->pinmask,status);
	}
}

void toggle_pin(pin_t* pin) {
	if(!pin->input) {
		set_pin(pin,!get_pin(pin));
	}
}

int main(void)
{
	//not sure thats even needed
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC,ENABLE);

	pin_t* led0 = create_pin(GPIOA,0,false);
	pin_t* led1 = create_pin(GPIOH,11,false);
	pin_t* led2 = create_pin(GPIOH,12,false);
	pin_t* led3 = create_pin(GPIOB,8,false);

	pin_t* tas0 = create_pin(GPIOC,7,true);
	pin_t* tas1 = create_pin(GPIOB,15,true);
	pin_t* tas2 = create_pin(GPIOB,14,true);
	pin_t* tas3 = create_pin(GPIOI,0,true);

	uint8_t old; //old state
	uint8_t cur; //current state
	uint8_t fla; //edge

	while(1) {

		//Aufgabe 1:
		//set_pin(led0,get_pin(tas0));
		//set_pin(led1,get_pin(tas1));
		//set_pin(led2,get_pin(tas2));
		//set_pin(led3,get_pin(tas3));


		//Aufgabe 2:
		cur = get_pin(tas0) | get_pin(tas1) << 1 | get_pin(tas2) << 2 | get_pin(tas3) << 3;
		fla = (cur ^ old) & cur; //detect positve edge
		old = cur;

		if(fla&1) toggle_pin(led0);
		if(fla&2) toggle_pin(led1);
		if(fla&4) toggle_pin(led2);
		if(fla&8) toggle_pin(led3);



	}

	destory_pin(led0);
	destory_pin(led1);
	destory_pin(led2);
	destory_pin(led3);
	destory_pin(tas0);
	destory_pin(tas1);
	destory_pin(tas2);
	destory_pin(tas3);

	return 0;
}

