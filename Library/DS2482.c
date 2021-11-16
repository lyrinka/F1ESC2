#include <stm32f10x.h>
#include <SoftI2C.h>
#include <Task.h>
#include "DS2482.h"

void DS2482_Init(unsigned char addr) {
	I2C_Write(addr, 1, 0xF0, 0, 0); // Reset
	I2C_Write(addr, 2, 0xD2E1, 0, 0); // Active Pullup
}

void DS2482_Polling_Delay(void) {
	await(0); // Skip cycle
}

unsigned char DS2482_1Wire_Polling(unsigned char addr) {
	for(;;) {
		unsigned char data; 
		I2C_Read(addr, 1, &data); 
		if(!(data & 0x01)) // 1-Wire Busy Bit
			return data; 
		DS2482_Polling_Delay(); 
	}
}

int DS2482_1Wire_Reset(unsigned char addr) {
	I2C_Write(addr, 1, 0xB4, 0, 0); 
	return !(DS2482_1Wire_Polling(addr) & 0x02); 
}

void DS2482_1Wire_Write(unsigned char addr, unsigned char data) {
	I2C_Write(addr, 1, 0xA5, 1, &data); 
	DS2482_1Wire_Polling(addr); 
}

unsigned char DS2482_1Wire_Read(unsigned char addr) {
	unsigned char data; 
	I2C_Write(addr, 1, 0x96, 0, 0); 
	DS2482_1Wire_Polling(addr); 
	I2C_WriteRead(addr, 2, 0xE1E1, 0, 0, 1, &data); 
	return data; 
}

void DS2482_1Wire_WriteBit(unsigned char addr, int bit) {
	unsigned int data = bit ? 0x8780 : 0x8700; 
	I2C_Write(addr, 2, data, 0, 0); 
	DS2482_1Wire_Polling(addr); 
}

int DS2482_1Wire_ReadBit(unsigned char addr) {
	I2C_Write(addr, 2, 0x8780, 0, 0); 
	unsigned char response = DS2482_1Wire_Polling(addr); 
	return (response & 0x20) ? 0x1 : 0x0; 
}

int DS2482_1Wire_Triplet(unsigned char addr, int bit) { // Return value: Direction Taken, Second Bit, First Bit
	unsigned int data = bit ? 0x7880 : 0x7800; 
	I2C_Write(addr, 2, data, 0, 0); 
	unsigned char response = DS2482_1Wire_Polling(addr); 
	return (response >> 5) & 0x7; 
}
