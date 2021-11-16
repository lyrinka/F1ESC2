#include <SoftI2C.h>
#include "PCF8574.h"

void PCF8574_Output(unsigned char addr, unsigned char value) {
	I2C_Write(addr, 1, value, 0, 0); 
}
