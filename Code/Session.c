#include <stm32f10x.h>
#include <Task.h>
#include "GPIO.h"
#include "CANBus.h"

#include "Motor.h"

// Core Logic
#define PacketOutFault 0x000
#define PacketOutSync 0x001
#define PacketOutMotor 0x002
#define PacketOutPower 0x003
#define PacketOutTemperature 0x004
#define PacketOutBattery 0x005


typedef struct {
	unsigned char dirty; 
	unsigned char mode; 
	unsigned short PWM; 
	unsigned short VoPercent; 
} Session_PendingMotorState_TypeDef; 

Session_PendingMotorState_TypeDef Session_PendingMotorState[4]; 

int Session_Task_ApplicationLogic(CAN_Packet_TypeDef * packet) {
	int error = 0; 
	switch(packet->id) {
		default: {
			error = 1; 
			break; 
		}
		case PacketOutFault: {
			if(packet->len != 0 && packet->len != 2) {
				error = 1; 
				break; 
			}
			if(packet->len == 2) {
				if(!Motor_Sync_Takeover) {
					Motor_Power_Control.ForceRun = packet->data[0] ? 1 : 0; 
					Motor_Power_Control.Fault &= ~packet->data[1]; 
				}
			}
			packet->len = 3; 
			packet->data[0] = Motor_Sync_Takeover ? 1 : 0; 
			packet->data[1] = Motor_Power_Control.ForceRun ? 1 : 0; 
			packet->data[2] = Motor_Power_Control.Fault; 
			break; 
		}
		case PacketOutSync: {
			if(packet->len != 0) {
				error = 1; 
				break; 
			}
			if(!Motor_Sync_Takeover) {
				for(int i = 0; i < 4; i++) {
					if(!Session_PendingMotorState[i].dirty) continue; 
					Session_PendingMotorState[i].dirty = 0; 
					Motor_Channel_Controls[i].Mode = Session_PendingMotorState[i].mode; 
					Motor_Channel_Controls[i].PWM = Session_PendingMotorState[i].PWM; 
					Motor_Channel_Controls[i].DesiredVoltage = Motor_CalcVoPercent(Session_PendingMotorState[i].VoPercent); 
				}
			}
			Motor_Sync_Received = 1; 
			unsigned char temp; 
			for(int i = 0; i < 4; i++) 
				temp |= Motor_VPID_Controller.LockIndicator[i] << i; 
			packet->len = 1; 
			packet->data[0] = temp; 
			break; 
		}
		case PacketOutMotor: {
			if(packet->len != 1 && packet->len != 6) {
				error = 1; 
				break; 
			}
			int index = packet->data[0] & 0x3; 
			if(packet->len == 6) {
				Session_PendingMotorState[index].dirty = 1; 
				Session_PendingMotorState[index].mode = packet->data[1] & 0x3; 
				Session_PendingMotorState[index].PWM = ((packet->data[2] << 8) | packet->data[3]); 
				short temp = ((packet->data[4] << 8) | packet->data[5]); 
				if(temp > 4095) temp = 4095; 
				if(temp < 0) temp = 0; 
				Session_PendingMotorState[index].VoPercent = temp; 
			}
			packet->len = 8; 
			packet->data[0] = Motor_Channel_Measurements[index].Voltage >> 8; 
			packet->data[1] = Motor_Channel_Measurements[index].Voltage & 0xFF; 
			packet->data[2] = Motor_Channel_Measurements[index].Current >> 8; 
			packet->data[3] = Motor_Channel_Measurements[index].Current & 0xFF; 
			packet->data[4] = Motor_Channel_Measurements[index].Speed >> 8; 
			packet->data[5] = Motor_Channel_Measurements[index].Speed & 0xFF; 
			packet->data[6] = Motor_Channel_Measurements[index].Temperature >> 8; 
			packet->data[7] = Motor_Channel_Measurements[index].Temperature & 0xFF; 
			break; 			
		}
		case PacketOutPower: {
			if(packet->len != 0 && packet->len != 1) {
				error = 1; 
				break; 
			}
			if(packet->len == 1) {
				if(!Motor_Sync_Takeover) {
					if(packet->data[0]) 
						Motor_Power_Control.Power = MOTOR_POWER_MASTER_SWITCH | MOTOR_POWER_SPEED_SENSOR | MOTOR_POWER_THERMISTOR | MOTOR_POWER_MOTOR_DRIVE | MOTOR_POWER_VPID_LOOP; 
					else 
						Motor_Power_Control.Power = 0x00; 
				}
			}
			packet->len = 8; 
			packet->data[0] = Motor_Power_Measurement.UPSVoltage >> 8; 
			packet->data[1] = Motor_Power_Measurement.UPSVoltage & 0xFF; 
			packet->data[2] = Motor_Power_Measurement.UPSCurrent >> 8; 
			packet->data[3] = Motor_Power_Measurement.UPSCurrent & 0xFF; 
			packet->data[4] = Motor_Power_Measurement.PWRVoltage >> 8; 
			packet->data[5] = Motor_Power_Measurement.PWRVoltage & 0xFF; 
			packet->data[6] = Motor_Power_Measurement.PWRCurrent >> 8; 
			packet->data[7] = Motor_Power_Measurement.PWRCurrent & 0xFF; 
			break; 
		}
		case PacketOutTemperature: {
			if(packet->len != 0) {
				error = 1; 
				break; 
			}
			packet->len = 8; 
			packet->data[0] = Motor_Thermistor_Measurement.Converter[0]; 
			packet->data[1] = Motor_Thermistor_Measurement.Switch[0]; 
			packet->data[2] = Motor_Thermistor_Measurement.Converter[1]; 
			packet->data[3] = Motor_Thermistor_Measurement.Switch[1]; 
			packet->data[4] = Motor_Thermistor_Measurement.Converter[2]; 
			packet->data[5] = Motor_Thermistor_Measurement.Switch[2]; 
			packet->data[6] = Motor_Thermistor_Measurement.Converter[3]; 
			packet->data[7] = Motor_Thermistor_Measurement.Switch[3]; 
			break; 
		}
		case PacketOutBattery: {
			if(packet->len != 0) {
				error = 1; 
				break; 
			}
			packet->len = 4; 
			packet->data[0] = Motor_Power_Measurement.UPSVoltage >> 8; 
			packet->data[1] = Motor_Power_Measurement.UPSVoltage & 0xFF; 
			packet->data[2] = Motor_Power_Measurement.BattTemperature >> 8; 
			packet->data[3] = Motor_Power_Measurement.BattTemperature & 0xFF; 
			break; 
		}
		
	}
	return error; 
}

void Session_Task_Func(void) {
	// Datastructure Initialization
	for(int i = 0; i < 4; i++) 
		Session_PendingMotorState[i].dirty = 0; 
	
	// Interface Initialization
	CAN_Init(); 
	
	// Main application loop
	while(1) {
		CAN_Packet_TypeDef CAN_Packet; 
		
		// Wait for packet
		CAN_BlockingReceive(&CAN_Packet); 
		
		// Process packet
		GPIO_LED_ACT_ON(); 
		int error = Session_Task_ApplicationLogic(&CAN_Packet); 
		
		// Transmit ack packet if no error
		if(!error) 
			CAN_BlockingTransmit(&CAN_Packet); 
		
		GPIO_LED_ACT_OFF(); 
	}
}
