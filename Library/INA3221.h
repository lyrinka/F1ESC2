#ifndef __INA3221_H__
#define __INA3221_H__

#define INA3221_REG_CONFIG 0x00
#define INA3221_REG_CH1A 0x01
#define INA3221_REG_CH1V 0x02
#define INA3221_REG_CH2A 0x03
#define INA3221_REG_CH2V 0x04
#define INA3221_REG_CH3A 0x05
#define INA3221_REG_CH3V 0x06
#define INA3221_REG_CH1CRIT 0x07
#define INA3221_REG_CH1WARN 0x08
#define INA3221_REG_CH2CRIT 0x09
#define INA3221_REG_CH2WARN 0x0A
#define INA3221_REG_CH3CRIT 0x0B
#define INA3221_REG_CH3WARN 0x0C
#define INA3221_REG_ASUM 0x0D
#define INA3221_REG_ASUML 0x0E
#define INA3221_REG_MASK 0x0F
#define INA3221_REG_PVUT 0x10
#define INA3221_REG_PVLT 0x11
#define INA3221_REG_MANUID 0xFE
#define INA3221_REG_CHIPID 0xFF

typedef struct {
	short CHxA[3]; 
	short CHxV[3]; 
} INA3221_Measurement_TypeDef; 

extern void INA3221_Init(unsigned char addr); 
extern void INA3221_Read_Measurement(unsigned char addr, INA3221_Measurement_TypeDef * data); 

#endif
