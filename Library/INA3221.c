#include <stm32f10x.h>
#include <SoftI2C.h>
#include "INA3221.h"

unsigned short INA3221_ReadReg(unsigned char addr, unsigned char reg) {
	unsigned short value; 
	I2C_WriteRead(addr, 1, reg, 0, 0, 2, (unsigned char *)&value); 
	return __REV16(value); 
}

void INA3221_WriteReg(unsigned char addr, unsigned char reg, unsigned short value) {
	value = __REV16(value); 
	I2C_Write(addr, 1, reg, 2, (unsigned char *)&value); 
}

void INA3221_Init(unsigned char addr) {
	INA3221_WriteReg(addr, INA3221_REG_CONFIG, 0xF127); // Reset
	INA3221_WriteReg(addr, INA3221_REG_CONFIG, 0x7427); // all 1.1ms, avgx16, cont. mode
	INA3221_WriteReg(addr, INA3221_REG_MASK, 0x0C00); // Both alerts latch
}

void INA3221_Read_Measurement(unsigned char addr, INA3221_Measurement_TypeDef * data) {
	data->CHxA[0] = ((short)INA3221_ReadReg(addr, INA3221_REG_CH1A)) >> 3; 
	data->CHxA[1] = ((short)INA3221_ReadReg(addr, INA3221_REG_CH2A)) >> 3; 
	data->CHxA[2] = ((short)INA3221_ReadReg(addr, INA3221_REG_CH3A)) >> 3; 
	data->CHxV[0] = ((short)INA3221_ReadReg(addr, INA3221_REG_CH1V)) >> 3; 
	data->CHxV[1] = ((short)INA3221_ReadReg(addr, INA3221_REG_CH2V)) >> 3; 
	data->CHxV[2] = ((short)INA3221_ReadReg(addr, INA3221_REG_CH3V)) >> 3; 
}
