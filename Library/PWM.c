#include <stm32f10x.h>
#include <Variables.h>
#include <Task.h>
#include "PWM.h"

volatile unsigned int PWM_CurrentCycle; 
volatile unsigned int PWM_UpdateFlag; 

void PWM_Init(void) {
	PWM_CurrentCycle = 0; 
	PWM_UpdateFlag = 0; 
	// PWM Timer (TIM1)
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; 
	__DSB(); 
	
	TIM1->ARR = 0x0FFF; // 12bit
	TIM1->PSC = 0x00AE; // 100.4Hz
	TIM1->RCR = 0x01; // Every 2 cycles 
	
	TIM1->CCMR1 = 0x6868; // Positive Polarity
	TIM1->CCMR2 = 0x6868; 
	TIM1->CCER = 0x1111; // No complementary outputs
	TIM1->BDTR = 0x1300; // Active low break
	TIM1->CR2 = 0x0000; 
	TIM1->CR1 = 0x0080; 
	TIM1->EGR = 0x0001; // Update
	TIM1->SR = 0x00; 
	
	NVIC_SetPriority(TIM1_UP_IRQn, PRIORITY_TIM1_UP); 
	NVIC_ClearPendingIRQ(TIM1_UP_IRQn); 
	NVIC_EnableIRQ(TIM1_UP_IRQn); 
	
	TIM1->DIER = 0x01; 
	TIM1->CCR1 = 0x0000; 
	TIM1->CCR2 = 0x0000; 
	TIM1->CCR3 = 0x0000; 
	TIM1->CCR4 = 0x0000; 
//TIM1->BDTR |= 0x8000; // MOE
	TIM1->CR1 |= 0x01; 
}

int PWM_PromiseCallback_Cycle(void) {
	return PWM_UpdateFlag; 
}

int PWM_BlockingWaitCycle(void) {
	Promise_TypeDef promise; 
	Promise_Set(&promise, PWM_PromiseCallback_Cycle); 
//	if(PWM_UpdateFlag) { // Loop Late
		PWM_UpdateFlag = 0; 
//		return 1; 
//	}
	await(&promise); 
	PWM_UpdateFlag = 0; 
	return 0; 
}

void TIM1_UP_IRQHandler(void) {
	PWM_CurrentCycle++; 
	PWM_UpdateFlag = 1; 
	TIM1->SR = ~0x01; 
}
