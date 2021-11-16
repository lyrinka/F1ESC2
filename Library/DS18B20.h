#ifndef __DS18B20_H__
#define __DS18B20_H__

#define DS18B20_CONFIG_9BIT  0x1F
#define DS18B20_CONFIG_10BIT 0x3F
#define DS18B20_CONFIG_11BIT 0x5F
#define DS18B20_CONFIG_12BIT 0x7F

typedef struct {
	unsigned char family; 
	unsigned char address[6]; 
	unsigned char crc; 
} DS18B20_ID_TypeDef; 

typedef struct {
	short temp; 
	char temp_high; 
	char temp_low; 
	unsigned char config; 
	unsigned char reserved[3]; 
	unsigned char crc; 
} DS18B20_Measurement_TypeDef; 

typedef struct {
	char temp_high; 
	char temp_low; 
	unsigned char config; 
} DS18B20_Config_TypeDef; 

extern int DS18B20_ConvertAllAsync(unsigned char); 
extern int DS18B20_ReadMeasurement(unsigned char, const DS18B20_ID_TypeDef *, DS18B20_Measurement_TypeDef *); 
extern int DS18B20_ReadID(unsigned char, DS18B20_ID_TypeDef *); 

#endif
