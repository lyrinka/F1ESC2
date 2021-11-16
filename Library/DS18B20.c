#include <stm32f10x.h>
#include "DS2482.h"
#include "DS18B20.h"

int DS18B20_ConvertAllAsync(unsigned char addr) {
	if(DS2482_1Wire_Reset(addr)) return 1; 
	DS2482_1Wire_Write(addr, 0xCC); 
	DS2482_1Wire_Write(addr, 0x44); 
	return 0; 
}

int DS18B20_ReadMeasurement(unsigned char addr, const DS18B20_ID_TypeDef * id, DS18B20_Measurement_TypeDef * meas) {
	if(DS2482_1Wire_Reset(addr)) return 1; 
	DS2482_1Wire_Write(addr, 0x55); 
	for(int i = 0; i < 8; i++) 
		DS2482_1Wire_Write(addr, ((unsigned char *)id)[i]); 
	DS2482_1Wire_Write(addr, 0xBE); 
	for(int i = 0; i < 9; i++) 
		((unsigned char *)meas)[i] = DS2482_1Wire_Read(addr); 
	for(int i = 0; i < 9; i++) 
		if(((unsigned char *)meas)[i] != 0xFF) 
			return 0; 
	return -1; 
}

int DS18B20_ReadID(unsigned char addr, DS18B20_ID_TypeDef * id) {
	if(DS2482_1Wire_Reset(addr)) return 1; 
	DS2482_1Wire_Write(addr, 0x33); 
	for(int i = 0; i < 8; i++) 
		((unsigned char *)id)[i] = DS2482_1Wire_Read(addr); 
	return 0; 
}
/*
int DS18B20_WriteConfig(unsigned char addr, DS18B20_Config_TypeDef * config) {
	if(DS2482_1Wire_Reset(addr)) return 1; 
	DS2482_1Wire_Write(addr, 0xCC); 
	DS2482_1Wire_Write(addr, 0x4E); 
	for(int i = 0; i < 3; i++) 
		DS2482_1Wire_Write(addr, ((unsigned char *)config)[i]); 
	if(DS2482_1Wire_Reset(addr)) return 1; 
	DS2482_1Wire_Write(addr, 0xCC); 
	DS2482_1Wire_Write(addr, 0x48); 
	return 0;
} */
