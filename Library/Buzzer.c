#include <stm32f10x.h>
#include <Variables.h>
#include <Task.h>
#include "Buzzer.h"

#define TIMER_FREQUENCY 72000000
#define TIMER_PRESCALER 11

volatile int Buzzer_CycleCount; 

Buzzer_Sequence_Typedef Buzzer_Sequence; 

void Buzzer_Init(void) {
	Buzzer_CycleCount = 0; 
	Buzzer_Sequence.index = 0; 
	Buzzer_Sequence.length = -1; 
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 
	
	TIM2->PSC = TIMER_PRESCALER - 1; 
	
	TIM2->CCMR1 = 0x0068; 
	TIM2->DIER = 0x0001; 
	TIM2->CR1 = 0x0084; 
	
	TIM2->ARR = 0xFFFF; 
	TIM2->CNT = 0x0000; 
	TIM2->CCR1 = 0x7FFF; 
	
	TIM2->EGR = 0x0001; 
	
	TIM2->SR = 0x0; 
	
	NVIC_SetPriority(TIM2_IRQn, PRIORITY_TIM2); 
	NVIC_ClearPendingIRQ(TIM2_IRQn); 
	NVIC_EnableIRQ(TIM2_IRQn); 
}

void Buzzer_Single_Load(int freq, int time) {
	// Freq in 0.1Hz
	// Time in ms
	if(freq <= 0) {
		freq = 1000; 
		TIM2->CCER &= ~0x01; 
	}
	else 
		TIM2->CCER |= 0x01; 
	if(freq < 1000) freq = 1000; 
	if(freq > 200000) freq = 200000; 
	if(time < 1) time = 1; 
	if(time > 600000) time = 600000; 
	int cycle = (TIMER_FREQUENCY * 10) / (freq * TIMER_PRESCALER); 
	int total = time * ((TIMER_FREQUENCY / 11) / 1000); 
	if(cycle == 0) cycle = 1; 
	int quotient = total / cycle; 
	int remainder = total - (quotient * cycle); 
	if(remainder > 0) {
		quotient++; 
		remainder = cycle - remainder; 
	}
	
	TIM2->CR1 &= ~0x01; // Stop the timer
	
	TIM2->ARR = cycle; 
	TIM2->CCR1 = cycle / 3; 
	
	Buzzer_CycleCount = quotient; 
	TIM2->EGR = 0x01; // Load all values
	TIM2->CNT = remainder; 
	TIM2->CR1 |= 0x01; // Start the timer
}

void Buzzer_Single_Stop(void) {
	TIM2->CR1 &= ~0x01; // Stop the timer
	TIM2->CCER &= ~0x01; // Disable Output
	Buzzer_CycleCount = 0; // Reset counter
}

void Buzzer_Abort(void) {
	Buzzer_Single_Stop(); 
	Buzzer_Sequence.index = 0; 
	Buzzer_Sequence.length = -1; 
}

void Buzzer_Trigger(void) {
	int index = Buzzer_Sequence.index; 
	int length = Buzzer_Sequence.length; 
	if(index >= length) {
		TIM2->CCER &= ~0x01; // Disable Output
		Buzzer_Sequence.index = 0; 
		Buzzer_Sequence.length = -1; // Finished
		return; 
	}
	int freq = Buzzer_Sequence.freq[index]; 
	int time = Buzzer_Sequence.time[index]; 
	Buzzer_Sequence.index = index + 1; 
	Buzzer_Single_Load(freq, time); 
}

int Buzzer_PromiseCallback_Wait(void) {
	return Buzzer_Sequence.length < 0; 
}

void Buzzer_Load(int length, const unsigned short * freq, const unsigned short * time) {
	Buzzer_Sequence.index = 0; 
	Buzzer_Sequence.length = length; 
	Buzzer_Sequence.freq = freq; 
	Buzzer_Sequence.time = time; 
}

void Buzzer_BlockingWait(void) {
	Promise_TypeDef promise; 
	Promise_Set(&promise, Buzzer_PromiseCallback_Wait); 
	await(&promise); 
}

void Buzzer_Play(int length, const unsigned short * freq, const unsigned short * time) {
	Buzzer_BlockingWait(); 
	Buzzer_Load(length, freq, time); 
	Buzzer_Trigger(); 
	Buzzer_BlockingWait(); 
}

void Buzzer_PlayAsync(int length, const unsigned short * freq, const unsigned short * time) {
	Buzzer_Abort(); 
	Buzzer_Load(length, freq, time); 
	Buzzer_Trigger(); 
}

void TIM2_IRQHandler(void) {
	TIM2->SR = ~0x01; 
	if(Buzzer_CycleCount < 0) return; 
	if(--Buzzer_CycleCount) return; 
	TIM2->CR1 &= ~0x01; // Stop the timer
	Buzzer_Trigger(); 
}
