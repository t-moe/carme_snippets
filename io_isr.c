#include <carme.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <stdlib.h>
#include <color.h>
#include <lcd.h>

/*******************************/

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
/*******************************/

pin_t* led0;
pin_t* led1;
pin_t* tas0;
pin_t* tas1;
volatile bool t0Down;
volatile bool t1Up;


void TIM6_DAC_IRQHandler(void) {
	if(TIM_GetFlagStatus(TIM6,TIM_FLAG_Update)) {
	    toggle_pin(led1);
		TIM_ClearFlag(TIM6,TIM_FLAG_Update);
	}
}

void EXTI9_5_IRQHandler(void) {
	if(EXTI_GetFlagStatus(EXTI_Line7)) {
		set_pin(led0,false);
		t0Down = true;
		EXTI_ClearFlag(EXTI_Line7);
	}
}

void EXTI15_10_IRQHandler(void) {
	if(EXTI_GetFlagStatus(EXTI_Line15)) {
		set_pin(led0,true);
		t1Up=true;
		EXTI_ClearFlag(EXTI_Line15);
	}
}

void init_gpio() {
	//Taster&Leds konfigurieren
	RCC_APB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC,ENABLE);
	led0 = create_pin(GPIOA,0,false);
	led1 = create_pin(GPIOH,11,false);

	tas0 = create_pin(GPIOC,7,true);
	tas1 = create_pin(GPIOB,15,true);
}

void init_exti() {
	//EXTI konfigurieren
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource7);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB,EXTI_PinSource15);

	EXTI_InitTypeDef ei;
	EXTI_StructInit(&ei);
	ei.EXTI_Line = EXTI_Line7;
	ei.EXTI_Mode = EXTI_Mode_Interrupt;
	ei.EXTI_LineCmd = ENABLE;
	ei.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&ei);

	EXTI_StructInit(&ei);
	ei.EXTI_Line = EXTI_Line15;
	ei.EXTI_Mode = EXTI_Mode_Interrupt;
	ei.EXTI_LineCmd = ENABLE;
	ei.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&ei);

	//Exti interrupt aktivieren und freigeben
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

}

void init_timer() {
	//Timer konfigurieren
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM6EN,ENABLE);
	const int APB1_CLK=42E6; //42mhz APB1 Takt
	TIM_TimeBaseInitTypeDef tinit;
	TIM_TimeBaseStructInit(&tinit);
	tinit.TIM_Prescaler = APB1_CLK/1000 -1; //0...41999
	tinit.TIM_Period = 1000-1; //counts from 0 to 999
	TIM_TimeBaseInit(TIM6,&tinit);


	//Timer isr aktivieren und interrupt freigeben
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);

	TIM_Cmd(TIM6,ENABLE);
}


int main(void)
{
	__disable_irq(); //globale isr aus

	init_gpio();
	init_exti();
	init_timer();

	__enable_irq(); //globale isr wieder an

	LCD_Init();
	LCD_Clear(GUI_COLOR_BLACK);
	t1Up = false;
	t0Down = false;


	while(1) {
		if(t0Down) {
			LCD_DisplayStringLine(0,"T0 Down");
			t0Down= false;
		}
		if(t1Up) {
			LCD_DisplayStringLine(0,"T1 Up");
			t1Up= false;
		}
	}

	destroy_pin(led0);
	destroy_pin(led1);
	destroy_pin(tas0);
	destroy_pin(tas1);

	return 0;
}

