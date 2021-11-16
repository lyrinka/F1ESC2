#include <stm32f10x.h>
#include <Variables.h>
#include "Tacho.h"

#define TACHO_IIR_DIVIDER 65455 // IIR Filter Time constant 0.1s
#define TACHO_IIR_TABLE_MAXIDX 31
const short TACHO_IIR_TABLE[32] = { // 95% IIR Filter after a rise time
	4096, 3891, 3180, 2587, 2159, 1846, 1610, 1426, 
	1279, 1160, 1060,  977,  905,  843,  789,  742, 
	 699,  662,  628,  597,  570,  545,  521,  500,
   481,  463,  446,  430,  416,  402,  389,  377, 
}; 

#define TACHO_CYCLETIMEOUT 2
#define TACHO_RPM_CONVERTER 19636364 // 2 pulses per revolution

volatile unsigned int Tacho_CurrentCycle; 
Tacho_Channels_TypeDef Tacho_Channels[4]; 
void (*Tacho_UpdateCallback)(void); 

void Tacho_Init(void) {
	for(int i = 0; i < 4; i++) {
		Tacho_Channels[i].cycle = 0; 
		Tacho_Channels[i].subcycle = 0; 
		Tacho_Channels[i].average = 0; 
		Tacho_Channels[i].interval = -1; 
		Tacho_Channels[i].rpm = -1; 
	}
	Tacho_UpdateCallback = 0; 
	
	// Tachometer Timer (TIM3)
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; 
	__DSB(); 
	
	TIM3->ARR = 0xFFFF; // 16bit
	TIM3->PSC = 109; // ~10Hz
	
	TIM3->CCMR1 = 0x2D2D; 
	TIM3->CCMR2 = 0x2D2D; 
	TIM3->CCER = 0x3333; 
	TIM3->CR2 = 0x0000; 
	TIM3->CR1 = 0x0000; 
	TIM3->EGR = 0x01; 
	TIM3->SR = 0x00; 
	
	NVIC_SetPriority(TIM3_IRQn, PRIORITY_TIM3); 
	NVIC_ClearPendingIRQ(TIM3_IRQn); 
	NVIC_EnableIRQ(TIM3_IRQn); 
	
	TIM3->DIER = 0x1F; // 4 channel capture irq, update irq
	TIM3->CR1 |= 0x01; 
}

void Tacho_SetCallback(void (*updatecallback)(void)) {
	Tacho_UpdateCallback = updatecallback; 
}

void TIM3_IRQHandler(void) {
	unsigned int curr_cycle = Tacho_CurrentCycle; // shadow
	unsigned int flags = TIM3->SR; 
	if(flags & 0x01) { // UIF
		// TODO: verify what will happen when at the same time
		curr_cycle = curr_cycle + 1; 
		Tacho_CurrentCycle = curr_cycle; 
		for(int i = 0; i < 4; i++) {
			int interval = Tacho_Channels[i].interval; 
			if(interval < 0) 
				Tacho_Channels[i].rpm = -1; 
			else 
				Tacho_Channels[i].rpm = TACHO_RPM_CONVERTER / interval; 
		}
		if(Tacho_UpdateCallback) Tacho_UpdateCallback(); 
		TIM3->SR = ~0x01; 
	}
	for(int i = 0; i < 4; i++) {
		unsigned int flag_mask = 1 << (i + 1); 
		if(!(flags & flag_mask)) continue; 
		int curr_subcycle = (int)(&TIM3->CCR1)[i << 1]; 
		int prev_interval = Tacho_Channels[i].interval; 
		if(prev_interval >= 0) {
			unsigned int prev_cycle = Tacho_Channels[i].cycle; 
			int prev_subcycle = (int)Tacho_Channels[i].subcycle; 
			// Perform IIR Filtering
			int interval = (curr_subcycle - prev_subcycle) + ((curr_cycle - prev_cycle) << 16); 
			Tacho_Channels[i].interval = prev_interval + (((interval - prev_interval) * Tacho_Channels[i].average) >> 12); 
			// Calculate rise time (in samples) for next IIR cycle
			int n_samples = TACHO_IIR_DIVIDER / interval; 
			if(n_samples > TACHO_IIR_TABLE_MAXIDX) n_samples = TACHO_IIR_TABLE_MAXIDX; 
			Tacho_Channels[i].average = TACHO_IIR_TABLE[n_samples]; 
		}
		else {
			Tacho_Channels[i].interval = TACHO_CYCLETIMEOUT << 16; 
			Tacho_Channels[i].average = TACHO_IIR_TABLE[0]; 
		}
		Tacho_Channels[i].cycle = curr_cycle; 
		Tacho_Channels[i].subcycle = curr_subcycle; 
		TIM3->SR = ~flag_mask; 
	}
	for(int i = 0; i < 4; i++) {
		if(curr_cycle - Tacho_Channels[i].cycle > TACHO_CYCLETIMEOUT) 
			Tacho_Channels[i].interval = -1; 
	}
}
