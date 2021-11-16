#include <Task.h>
#include <SoftI2C.h>
#include "LIS3DH.h"

#define LIS3DH_REG_AUTOINCREASE		0x80
#define LIS3DH_REG_STATUS_REG_AUX	0x07
#define LIS3DH_REG_OUT_ADC1_L			0x08
#define LIS3DH_REG_OUT_ADC1_H			0x09
#define LIS3DH_REG_OUT_ADC2_L			0x0A
#define LIS3DH_REG_OUT_ADC2_H			0x0B
#define LIS3DH_REG_OUT_ADC3_L			0x0C
#define LIS3DH_REG_OUT_ADC3_H			0x0D
#define LIS3DH_REG_WHO_AM_I				0x0F
#define LIS3DH_REG_CTRL_REG0			0x1E
#define LIS3DH_REG_TEMP_CFG_REG		0x1F
#define LIS3DH_REG_CTRL_REG1			0x20
#define LIS3DH_REG_CTRL_REG2			0x21
#define LIS3DH_REG_CTRL_REG3			0x22
#define LIS3DH_REG_CTRL_REG4			0x23
#define LIS3DH_REG_CTRL_REG5			0x24
#define LIS3DH_REG_CTRL_REG6			0x25
#define LIS3DH_REG_REFERENCE			0x26
#define LIS3DH_REG_STATUS_REG			0x27
#define LIS3DH_REG_OUT_X_L				0x28
#define LIS3DH_REG_OUT_X_H				0x29
#define LIS3DH_REG_OUT_Y_L				0x2A
#define LIS3DH_REG_OUT_Y_H				0x2B
#define LIS3DH_REG_OUT_Z_L				0x2C
#define LIS3DH_REG_OUT_Z_H				0x2D
#define LIS3DH_REG_FIFO_CTRL_REG	0x2E
#define LIS3DH_REG_FIFO_SRC_REG		0x2F
#define LIS3DH_REG_INT1_CFG				0x30
#define LIS3DH_REG_INT1_SRC				0x31
#define LIS3DH_REG_INT1_THS				0x32
#define LIS3DH_REG_INT1_DUR				0x33
#define LIS3DH_REG_INT2_CFG				0x34
#define LIS3DH_REG_INT2_SRC				0x35
#define LIS3DH_REG_INT2_THS				0x36
#define LIS3DH_REG_INT2_DUR				0x37
#define LIS3DH_REG_CLICK_CFG			0x38
#define LIS3DH_REG_CLICK_SRC			0x39
#define LIS3DH_REG_CLICK_THS			0x3A
#define LIS3DH_REG_TIME_LIMIT			0x3B
#define LIS3DH_REG_TIME_LATENCY		0x3C
#define LIS3DH_REG_TIME_WINDOW		0x3D
#define LIS3DH_REG_ACT_THS				0x3E
#define LIS3DH_REG_ACT_DUR				0x3F

void LIS3DH_WriteReg(unsigned char devaddr, unsigned char reg, unsigned char val) {
	I2C_Write(devaddr, 1, reg, 1, &val); 
}

unsigned char LIS3DH_ReadReg(unsigned char devaddr, unsigned char reg) {
	unsigned char val; 
	I2C_WriteRead(devaddr, 1, reg, 0, 0, 1, &val); 
	return val; 
}

short LIS3DH_ReadData(unsigned char devaddr, unsigned char reg) {
	short val; 
	I2C_WriteRead(devaddr, 1, reg | LIS3DH_REG_AUTOINCREASE, 0, 0, 2, (unsigned char *)&val); 
	return val; 
}

void LIS3DH_Init(unsigned char devaddr) {
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_CTRL_REG1	, 0x27); // 10Hz Normal (12bit)
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_CTRL_REG2	, 0x00); // No Filter
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_CTRL_REG3	, 0x40); // INT1 pin on IRQ1
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_CTRL_REG4	, 0x80); // Block Update, Scale 2g
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_CTRL_REG5	, 0x00); 
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_CTRL_REG6	, 0x00); // Active High (Normal High)
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_TEMP_CFG_REG, 0xC0); // Enable ADC & Temp sensor
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_INT1_THS	, 0x08); // 0.13g
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_INT1_DUR	, 0x14); // 2s
	LIS3DH_WriteReg(devaddr, LIS3DH_REG_INT1_CFG	, 0x0A); // XH or YH
}

short LIS3DH_Read_Temperature(unsigned char devaddr) { // LSB 0.25 degrees
	unsigned short data = (unsigned short)LIS3DH_ReadData(devaddr, LIS3DH_REG_OUT_ADC3_L); 
	short temp = (char)(data >> 6) + 65; 
	return temp; 
}
