#ifndef __IBUS_H__
#define __IBUS_H__

#define IBUS_SENSOR_UNKNOWN 0xFF
#define IBUS_SENSOR_INTVOLT 0x00 // a / 100 (V)
#define IBUS_SENSOR_TEMP    0x01 // a / 10 - 40 (C)
#define IBUS_SENSOR_RPM     0x02 // a (rpm)
#define IBUS_SENSOR_EXTVOLT 0x03 // a / 100 (V)
#define IBUS_SENSOR_TXVOLT  0x7F // a / 100 (V)
#define IBUS_SENSOR_PKTERR  0xFE // a (%)

#define IBUS_SENSOR_COUNT 15

extern void IBus_Init(void); 

typedef struct {
	int index; 
	int valid; 
	unsigned char buffer[32]; 
	unsigned short channel[14]; 
} IBus_Control_Data_TypeDef; 

typedef struct {
	int enable; 
	int index; 
	int txlen; 
	unsigned char buffer[6]; 
} IBus_Telemetry_Data_TypeDef; 

extern IBus_Control_Data_TypeDef IBus_Control_Data; 
extern IBus_Telemetry_Data_TypeDef IBus_Telemetry_Data; 

extern unsigned char IBus_Sensor_Type[IBUS_SENSOR_COUNT]; 
extern unsigned short IBus_Sensor_Data[IBUS_SENSOR_COUNT]; 

#endif
