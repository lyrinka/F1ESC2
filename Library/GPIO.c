#include <stm32f10x.h>

void GPIO_Init(void) {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN; 
	__DSB(); 
	AFIO->MAPR = 0x02004B04; 
	GPIOA->ODR = 0x30FF; 
	GPIOA->CRL = 0x88888988; 
	GPIOA->CRH = 0xA882AAAA; 
	GPIOB->ODR = 0x2C84; 
	GPIOB->CRL = 0x89448844; 
	GPIOB->CRH = 0x22286694; 
	GPIOC->ODR = 0x0000; 
	GPIOC->CRL = 0x00000000; 
	GPIOC->CRH = 0x42200000; 
}
