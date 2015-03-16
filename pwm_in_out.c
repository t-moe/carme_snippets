#include <carme.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <color.h>
#include <lcd.h>


void init_gpio() {
	//Taster&Leds konfigurieren
	RCC_APB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef t;
	GPIO_StructInit(&t);

	t.GPIO_Mode = GPIO_Mode_AF; //alternate function
	t.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//Input
	t.GPIO_OType = GPIO_OType_OD;
	t.GPIO_Speed = GPIO_Speed_50MHz;
	t.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB,&t);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_TIM4);

	//Output
	t.GPIO_OType = GPIO_OType_PP;
	t.GPIO_Speed = GPIO_Speed_100MHz;
	t.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA,&t);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5);
}

void init_timer(){
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM4EN,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM5EN,ENABLE);
	TIM_TimeBaseInitTypeDef tbi;
	TIM_OCInitTypeDef toi;
	TIM_ICInitTypeDef tii;
	TIM_TimeBaseStructInit(&tbi);
	TIM_OCStructInit(&toi);
	TIM_ICStructInit(&tii);


	//PWM output with TIM5 (100khz/10us)
	tbi.TIM_Prescaler = 839; //Goal CK_CNT=100kHz. Formula PSC= (CK_PSC-CK_CNT)/CK_CNT = (84mhz-100khz)/100khz=839
	tbi.TIM_Period = 9999; //Goal outer PWM_Freq=10Hz Formula TIM_Period= CK_CNT/PWM_Freq -1 = 100khz/10hz-1=9999
	tbi.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5,&tbi);
	TIM_Cmd(TIM5,ENABLE);

	//output capture/compare stuff
	toi.TIM_OCMode = TIM_OCMode_PWM1; //set bei compare match?
	toi.TIM_OutputState = TIM_OutputState_Enable;
	toi.TIM_Pulse = 1; //10us pulsewidth = 1 cnt pulse
	toi.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM5,&toi);
	TIM_OC1PreloadConfig(TIM5,TIM_OCPreload_Enable);

	//PWM input with TIM4 (500khz/2us)
	tbi.TIM_Prescaler = 167; //Goal CK_CNT=500kHz. Formula PSC= (CK_PSC-CK_CNT)/CK_CNT = (84mhz-500khz)/500khz=167
	tbi.TIM_Period = 0xFFFF; //count to max
	tbi.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4,&tbi);

	//input capture stuff. CH1 measures timervalue(period)
	tii.TIM_Channel = TIM_Channel_1;
	tii.TIM_ICPolarity = TIM_ICPolarity_Rising;
	tii.TIM_ICSelection = TIM_ICSelection_DirectTI; //direct from tim4
	tii.TIM_ICPrescaler = 0; //unused
	tii.TIM_ICFilter = 0; //unused?
	TIM_ICInit(TIM4,&tii);

	//CH2 measures cntvalue(pulsewidth)
	tii.TIM_Channel = TIM_Channel_2;
	tii.TIM_ICPolarity = TIM_ICPolarity_Falling;
	tii.TIM_ICSelection = TIM_ICSelection_IndirectTI; //indirect from tim4
	tii.TIM_ICPrescaler = 0; //unused
	tii.TIM_ICFilter = 0; //unused?
	TIM_ICInit(TIM4,&tii);

	TIM_SelectInputTrigger(TIM4,TIM_TS_TI1FP1);
	TIM_SelectSlaveMode(TIM4,TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIM4,TIM_MasterSlaveMode_Enable);
	TIM_Cmd(TIM4,ENABLE);
}

void init_isr(){
	TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);
	TIM_ITConfig(TIM4,TIM_IT_CC2,ENABLE);
	NVIC_EnableIRQ(TIM4_IRQn);
}

#define NUM_SAMPLES 16
volatile uint16_t cc2_arr[NUM_SAMPLES];
volatile uint8_t cc2_arr_ind=0;
volatile uint16_t cc1 =0;
volatile uint16_t cc2 = 0;
volatile bool update=false;

void TIM4_IRQHandler(void) {
	if(TIM_GetITStatus(TIM4,TIM_IT_CC1)==SET) {
		TIM_ClearITPendingBit(TIM4,TIM_IT_CC1);
		cc1 = TIM_GetCapture1(TIM4);
		update=true;
	}
	else if(TIM_GetITStatus(TIM4,TIM_IT_CC2)==SET) {
		TIM_ClearITPendingBit(TIM4,TIM_IT_CC2);
		cc2 = TIM_GetCapture2(TIM4);
		cc2_arr[cc2_arr_ind++]=cc2;
		cc2_arr_ind%=NUM_SAMPLES;
		update=true;
	}

}


int main(void)
{
	__disable_irq(); //globale isr aus

	init_gpio();
	init_timer();
	init_isr();

	__enable_irq(); //globale isr wieder an

	LCD_Init();
	LCD_Clear(GUI_COLOR_BLACK);

	char buf[30];

	while(1) {
		while(!update);
		update=false;
		sprintf(buf,"Period: %d",cc1);
		LCD_DisplayStringLine(0,buf);
		sprintf(buf,"Pulse: %d",cc2);
		LCD_DisplayStringLine(1,buf);
		sprintf(buf,"Echotime: %d us",cc2*2);
		LCD_DisplayStringLine(2,buf);
		sprintf(buf,"Distance %d cm",cc2*2/58);
		LCD_DisplayStringLine(2,buf);

		uint64_t i_sum=0;
		int i;
		for(i=0; i<NUM_SAMPLES; i++) {
			i_sum+=cc2_arr[i];
		}
		i_sum/=NUM_SAMPLES;
		sprintf(buf,"Distance smoothed %d cm",i_sum*2/58);
		LCD_DisplayStringLine(3,buf);


	}
	return 0;
}

