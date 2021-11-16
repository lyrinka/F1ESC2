#include <SoftI2C.h>
#include "ADS7830.h"

unsigned char ADS7830_Read(unsigned char addr, unsigned char ch) {
	unsigned char result; 
	I2C_WriteRead(addr, 1, (ch << 4) | 0x04, 0, 0, 1, &result); 
	return result; 
}
