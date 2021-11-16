#include <SoftI2C.h>
#include "MCP4728.h"

// All voltages are in 0.5mV steps. 

void MCP4728_SetChannelVoltage(unsigned char addr, int channel, int voltage) {
	unsigned int data = 0x418000 | ((channel & 0x3) << 17) | (voltage & 0xFFF); 
	I2C_Write(addr, 3, data, 0, 0); 
}

void MCP4728_SetChannelHigh(unsigned char addr, int channel) {
	unsigned int data = 0x410FFF | ((channel & 0x3) << 17); 
	I2C_Write(addr, 3, data, 0, 0); 

}

void MCP4728_SetChannelShut(unsigned char addr, int channel) {
	unsigned int data = 0x416000 | ((channel & 0x3) << 17); 
	I2C_Write(addr, 3, data, 0, 0); 
}
