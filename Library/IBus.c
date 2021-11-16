#include <stm32f10x.h>
#include <Variables.h>
#include "IBus.h"
// IBus Control Input: U2Rx
// IBus Telemetry Input: U1Tx (Half Duplex)

#define IBUS_TELEMETRY_PACKET_DISCOVER 0x80
#define IBUS_TELEMETRY_PACKET_READTYPE 0x90
#define IBUS_TELEMETRY_PACKET_READMEAS 0xA0

IBus_Control_Data_TypeDef IBus_Control_Data; 
IBus_Telemetry_Data_TypeDef IBus_Telemetry_Data; 

unsigned char IBus_Sensor_Type[IBUS_SENSOR_COUNT]; 
unsigned short IBus_Sensor_Data[IBUS_SENSOR_COUNT]; 

void IBus_Init(void) {
	IBus_Control_Data.index = 0; 
	IBus_Control_Data.valid = 0; 
	
	IBus_Telemetry_Data.enable = 0; 
	IBus_Telemetry_Data.index = 0; 
	IBus_Telemetry_Data.txlen = -1; 
	
	for(int i = 0; i < IBUS_SENSOR_COUNT; i++) 
		IBus_Sensor_Type[i] = IBUS_SENSOR_UNKNOWN; 
	
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN; 
	__DSB(); 
	
	USART1->BRR = 625; // baud 115200 
	USART1->SR = 0x0; 
	USART1->CR3 = 0x0000; 
	USART1->CR2 = 0x0000; 
	USART1->CR1 = 0x2024; 
	
	NVIC_SetPriority(USART1_IRQn, PRIORITY_USART1); 
	NVIC_ClearPendingIRQ(USART1_IRQn); 
	NVIC_EnableIRQ(USART1_IRQn); 
	
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; 
	__DSB(); 
	
	USART2->BRR = 313; // baud 115015
	USART2->SR = 0x0; 
	USART2->CR3 = 0x0008; // Half duplex
	USART2->CR2 = 0x0000; 
	USART2->CR1 = 0x202C; 
	
	NVIC_SetPriority(USART2_IRQn, PRIORITY_USART2); 
	NVIC_ClearPendingIRQ(USART2_IRQn); 
	NVIC_EnableIRQ(USART2_IRQn);
}

void USART1_IRQHandler(void) {
	volatile int dummyread = USART1->SR; 
	unsigned char ch = USART1->DR; 
	if(IBus_Control_Data.valid > 0) return; // Already have data
	int index = IBus_Control_Data.index; 
	if((index == 0) && (ch != 0x20)) return; 
	if((index == 1) && (ch != 0x40)) {
		IBus_Control_Data.index = 0; 
		return; 
	}
	IBus_Control_Data.buffer[index] = ch; 
	IBus_Control_Data.index = index + 1; 
	if(index < 31) return; 
	IBus_Control_Data.index = 0; 
	// Checksum
	int sum = 0xFFFF; 
	for(int i = 0; i < 31; i++) 
		sum -= IBus_Control_Data.buffer[i]; 
	sum -= IBus_Control_Data.buffer[31] << 8; 
	if(sum != 0) return; 
	// Copy data
	for(int i = 0; i < 14; i++) 
		IBus_Control_Data.channel[i] = ((unsigned short *)(IBus_Control_Data.buffer))[i + 1]; 
	if(IBus_Control_Data.valid >= 0) 
		IBus_Control_Data.valid = 1; 
}

void USART2_TxHandler(void) {
	int index = IBus_Telemetry_Data.index; 
	if(index >= IBus_Telemetry_Data.txlen) {
		IBus_Telemetry_Data.index = 0; 
		IBus_Telemetry_Data.txlen = -1; 
		// Clear TxEIE
		USART2->CR1 &= ~0x80; 
		return; 
	}
	USART2->DR = IBus_Telemetry_Data.buffer[index]; 
	IBus_Telemetry_Data.index = index + 1; 
}

void USART2_RxHandler(void) {
	volatile int dummyread = USART2->SR; 
	unsigned char ch = USART2->DR;
	if(IBus_Telemetry_Data.txlen >= 0) return; // Packet transmission is ongoing
	int index = IBus_Telemetry_Data.index; 
	if((index == 0) && (ch != 0x04)) return; 
	IBus_Telemetry_Data.buffer[index] = ch; 
	IBus_Telemetry_Data.index = index + 1; 
	if(index < 3) return; 
	IBus_Telemetry_Data.index = 0; 
	// Checksum
	int rxsum = 0xFFFF; 
	for(int i = 0; i < 3; i++) 
		rxsum -= IBus_Telemetry_Data.buffer[i]; 
	rxsum -= IBus_Telemetry_Data.buffer[3] << 8; 
	if(rxsum != 0) return; 
	// Process packet
	if(!IBus_Telemetry_Data.enable) return; 
	unsigned char packet = IBus_Telemetry_Data.buffer[1]; 
	unsigned char addr = (packet & 0x0F) - 1; 
	int txlen = -1; 
	switch(packet & 0xF0) {
		default: 
			break; 
		case IBUS_TELEMETRY_PACKET_DISCOVER: {
			if(IBus_Sensor_Type[addr] == IBUS_SENSOR_UNKNOWN) 
				break; 
			txlen = 4; 
			break; 
		}
		case IBUS_TELEMETRY_PACKET_READTYPE: {
			int type = IBus_Sensor_Type[addr]; 
			if(type == IBUS_SENSOR_UNKNOWN) break; 
			txlen = 6; 
			IBus_Telemetry_Data.buffer[2] = (unsigned char)type; 
			IBus_Telemetry_Data.buffer[3] = 0x02; 
			break; 
		}
		case IBUS_TELEMETRY_PACKET_READMEAS: {
			if(IBus_Sensor_Type[addr] == IBUS_SENSOR_UNKNOWN) 
				break; 
			txlen = 6; 
			unsigned short data = IBus_Sensor_Data[addr]; 
			IBus_Telemetry_Data.buffer[2] = data; 
			IBus_Telemetry_Data.buffer[3] = data >> 8; 
			break; 
		}
	}
	if(txlen < 0) return; 
	// Packet construction
	IBus_Telemetry_Data.txlen = txlen; 
	IBus_Telemetry_Data.buffer[0] = txlen; 
	// Checksum
	int txsum = 0xFFFF; 
	txlen -= 2; 
	for(int i = 0; i < txlen; i++) 
		txsum -= IBus_Telemetry_Data.buffer[i]; 
	IBus_Telemetry_Data.buffer[txlen] = txsum & 0xFF; 
	IBus_Telemetry_Data.buffer[txlen + 1] = txsum >> 8; 
	// Set TxEIE
	USART2->CR1 |= 0x80; 
}

void USART2_IRQHandler(void) {
	int status = USART2->SR; 
	if(status & 0x20) // RxNE
		USART2_RxHandler(); 
	if((USART2->CR1 & 0x80) && (status & 0x80)) // TxE
		USART2_TxHandler(); 
}

