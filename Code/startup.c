#include <stm32f10x.h>

#define OSC_TIMEOUT 400

int RESET_FLAGS; 
int HCLK_FREQ; 
int AHB_FREQ; 
int APB1_FREQ; 
int APB2_FREQ; 

int Startup_SetSysClockHS(void); 
void Startup_SetSysClockLS(void); 
void Startup_SetSysBkpDomain(int); 

void Startup(void) {
	// Fetch reset flags
	RESET_FLAGS = RCC->CSR; 
	// Clear reset flags & Enable LSI
	RCC->CSR = 0x01000001; 
	
	Startup_SetSysClockHS(); 
	Startup_SetSysBkpDomain(1); 
}

int Startup_SetSysClockHS(void) {
	// Configure 72MHz clock system
	RCC->CFGR = 0x001D8400; 
	RCC->CR = 0x01090083; 
	int clock_failed = 0; 
	int osc_timeout = OSC_TIMEOUT; 
	while(~(RCC->CR | 0xFDF4FFFC)) {
		if(--osc_timeout) continue; 
		RCC->CFGR = 0x00000000; 
		RCC->CR = 0x00000083; 
		clock_failed = 1; 
		break; 
	}
	
	// Switch system clock
	if(!clock_failed) {
		FLASH->ACR = 0x12; 
		RCC->CFGR |= 0x02; 
		while((RCC->CFGR & 0x0F) != 0x0A); 
		HCLK_FREQ = 72000000; 
		AHB_FREQ  = 72000000; 
		APB1_FREQ = 36000000; 
		APB2_FREQ = 72000000; 
	}
	else {
		HCLK_FREQ = 8000000; 
		AHB_FREQ  = 8000000; 
		APB1_FREQ = 8000000; 
		APB2_FREQ = 8000000; 
	}
	
	return clock_failed; 
}

void Startup_SetSysClockLS(void) {
	RCC->CFGR = 0x00000000; 
	RCC->CR = 0x00000083; 
	while(RCC->CFGR & 0x0F); 
	HCLK_FREQ = 8000000; 
	AHB_FREQ  = 8000000; 
	APB1_FREQ = 8000000; 
	APB2_FREQ = 8000000; 
}

void Startup_SetSysBkpDomain(int reset) {
	RCC->APB1ENR |= RCC_APB1ENR_BKPEN | RCC_APB1ENR_PWREN; 
	__DSB(); 
	// Disable BDWP
	PWR->CR = 0x100; 
	if(reset) RCC->BDCR = 0x10000; 
}
