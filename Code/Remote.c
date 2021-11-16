#include <stm32f10x.h>
#include <Task.h>
#include "IBus.h"
#include "Remote.h"

#include "Motor.h"

const unsigned short Thermistor_LookupTable[256] = {
	 390, 3350, 2832, 2570, 2399, 2274, 2176, 2096, 
	2029, 1971, 1920, 1875, 1835, 1798, 1765, 1734, 
	1705, 1679, 1654, 1630, 1609, 1588, 1568, 1549, 
	1532, 1515, 1498, 1483, 1468, 1454, 1440, 1426, 
	1413, 1401, 1389, 1377, 1366, 1355, 1344, 1334, 
	1324, 1314, 1304, 1295, 1285, 1276, 1268, 1259, 
	1251, 1242, 1234, 1226, 1219, 1211, 1203, 1196, 
	1189, 1182, 1175, 1168, 1161, 1154, 1148, 1141, 
	1135, 1129, 1123, 1116, 1110, 1104, 1098, 1093, 
	1087, 1081, 1076, 1070, 1065, 1059, 1054, 1048, 
	1043, 1038, 1033, 1028, 1023, 1018, 1013, 1008, 
	1003,  998,  993,  988,  984,  979,  974,  970, 
	 965,  960,  956,  951,  947,  942,  938,  934, 
	 929,  925,  921,  916,  912,  908,  903,  899, 
	 895,  891,  887,  883,  879,  874,  870,  866, 
	 862,  858,  854,  850,  846,  842,  838,  834, 
	 830,  826,  822,  818,  814,  811,  807,  803, 
	 799,  795,  791,  787,  783,  779,  776,  772, 
	 768,  764,  760,  756,  752,  749,  745,  741, 
	 737,  733,  729,  725,  722,  718,  714,  710, 
	 706,  702,  698,  694,  690,  686,  682,  679, 
	 675,  671,  667,  663,  659,  655,  651,  647, 
	 642,  638,  634,  630,  626,  622,  618,  614, 
	 609,  605,  601,  597,  592,  588,  583,  579, 
	 575,  570,  566,  561,  557,  552,  547,  543, 
	 538,  533,  528,  523,  518,  513,  508,  503, 
	 498,  493,  488,  482,  477,  471,  466,  460, 
	 454,  448,  442,  436,  430,  424,  417,  411, 
	 404,  397,  390,  383,  376,  368,  360,  352, 
	 344,  336,  327,  318,  309,  299,  289,  278, 
	 267,  255,  243,  230,  216,  201,  185,  168, 
	 149,  127,  103,   76,   43,    2,    0,    0, 
}; 

/* Sensor Display Order: 
		Internal Voltage
		Temperature
		Revolution Speed
		External Voltage
		(Local Battery Voltage)
		(Packet Error)
*/
const unsigned char Motor_Telemetry_Sensor_Type[IBUS_SENSOR_COUNT] = {
//IBUS_SENSOR_INTVOLT, 	// 5V Rail Voltage
	IBUS_SENSOR_INTVOLT, 	// UPS Voltage
	IBUS_SENSOR_TEMP, 		// Battery Temperature
	IBUS_SENSOR_TEMP, 		// Ambient Temperature
	IBUS_SENSOR_TEMP, 		// On-board Temperature (max)
	
	IBUS_SENSOR_TEMP, 		// Motor Temperature (max)
	IBUS_SENSOR_RPM, 			// Motor Speed (A)
	IBUS_SENSOR_RPM, 			// Motor Speed (B)
	IBUS_SENSOR_RPM, 			// Motor Speed (C)
	IBUS_SENSOR_RPM, 			// Motor Speed (D)
	
	IBUS_SENSOR_EXTVOLT, 	// Motor Voltage (Supply)
	IBUS_SENSOR_EXTVOLT, 	// Motor Voltage (A)
	IBUS_SENSOR_EXTVOLT, 	// Motor Voltage (B)
	IBUS_SENSOR_EXTVOLT, 	// Motor Voltage (C)
	IBUS_SENSOR_EXTVOLT, 	// Motor Voltage (D)
	
//IBUS_SENSOR_TXVOLT, 	// Transmitter Battery
//IBUS_SENSOR_PKTERR, 	// Packet Error Rate
	IBUS_SENSOR_PKTERR, 	// Fault Report
}; 

volatile int Remote_Telemetry_Ready; 

Remote_State_TypeDef Remote_State; 

#define CONVERT_VOLTAGE_SCALE_FROM_INA(v) ((unsigned short)((v) * 4 / 5))
#define CONVERT_TEMP_SCALE_FROM_DS18B20(t) ((t < -640) ? 0 : ((unsigned short)((t) * 5 / 8 + 400)))
#define CONVERT_TEMP_SCALE_FROM_THERMISTOR(t) (Thermistor_LookupTable[(unsigned char)t])
#define CONVERT_TEMP_SCALE_FROM_LIS3DH(t) ((t) * 5 / 2 + 400)
#define CONVERT_SPEED_SCALE(s) ((unsigned short)(((s) < 0) ? 0 : (s)))
void Remote_Callback_Update_Telemetry(void) {
	if(!Remote_Telemetry_Ready) return; 
	short motor_max_temp = -32768; 
	for(int i = 0; i < 4; i++) {
		short temp = Motor_Channel_Measurements[i].Temperature; 
		if(temp > motor_max_temp) motor_max_temp = temp; 
	}
	
	unsigned char board_max_temp; 
	if(Motor_Thermistor_Measurement.Valid) {
		board_max_temp = 255; 
		for(int i = 0; i < 4; i++) {
			unsigned char temp; 
			temp = Motor_Thermistor_Measurement.Converter[i]; 
			if(temp < board_max_temp) board_max_temp = temp; 
			temp = Motor_Thermistor_Measurement.Switch[i]; 
			if(temp < board_max_temp) board_max_temp = temp; 
		}
	}
	else board_max_temp = 0; 
	
	int fault = 1000; 
	if(Motor_Power_Control.ForceRun) 
		fault += 10000; 
	if(Motor_Power_Control.Fault) 
		fault += 1000; 
	if(Motor_Power_Control.Fault & MOTOR_FAULT_LOSESYNC) 
		fault += 1; 
	if(Motor_Power_Control.Fault & (MOTOR_FAULT_OVERCURRENT | MOTOR_FAULT_OVERTEMP | MOTOR_FAULT_HOTBATTERY | MOTOR_FAULT_UNDERVOLTAGE)) 
		fault += 10; 
	if(Motor_Power_Control.Fault & (MOTOR_FAULT_TILT | MOTOR_FAULT_STUCK)) 
		fault += 100; 
	
//IBus_Sensor_Data[  ]
	IBus_Sensor_Data[ 0] = CONVERT_VOLTAGE_SCALE_FROM_INA(Motor_Power_Measurement.UPSVoltage); 
	IBus_Sensor_Data[ 1] = CONVERT_TEMP_SCALE_FROM_DS18B20(Motor_Power_Measurement.BattTemperature); 
	IBus_Sensor_Data[ 2] = CONVERT_TEMP_SCALE_FROM_LIS3DH(Motor_Power_Measurement.AmbientTemperature); 
	IBus_Sensor_Data[ 3] = CONVERT_TEMP_SCALE_FROM_THERMISTOR(board_max_temp); 
	
	IBus_Sensor_Data[ 4] = CONVERT_TEMP_SCALE_FROM_DS18B20(motor_max_temp); 
	IBus_Sensor_Data[ 5] = CONVERT_SPEED_SCALE(Motor_Channel_Measurements[0].Speed); 
	IBus_Sensor_Data[ 6] = CONVERT_SPEED_SCALE(Motor_Channel_Measurements[1].Speed); 
	IBus_Sensor_Data[ 7] = CONVERT_SPEED_SCALE(Motor_Channel_Measurements[2].Speed); 
	IBus_Sensor_Data[ 8] = CONVERT_SPEED_SCALE(Motor_Channel_Measurements[3].Speed); 
	
	IBus_Sensor_Data[ 9] = CONVERT_VOLTAGE_SCALE_FROM_INA(Motor_Power_Measurement.PWRVoltage); 
	IBus_Sensor_Data[10] = CONVERT_VOLTAGE_SCALE_FROM_INA(Motor_Channel_Measurements[0].Voltage); 
	IBus_Sensor_Data[11] = CONVERT_VOLTAGE_SCALE_FROM_INA(Motor_Channel_Measurements[1].Voltage); 
	IBus_Sensor_Data[12] = CONVERT_VOLTAGE_SCALE_FROM_INA(Motor_Channel_Measurements[2].Voltage); 
	IBus_Sensor_Data[13] = CONVERT_VOLTAGE_SCALE_FROM_INA(Motor_Channel_Measurements[3].Voltage); 
	
//IBus_Sensor_Data[  ]
//IBus_Sensor_Data[  ]
	IBus_Sensor_Data[14] = fault; 

	if(!IBus_Telemetry_Data.enable) IBus_Telemetry_Data.enable = 1; 
}

int Remote_PromiseCallback_DataReady(void) {
	return IBus_Control_Data.valid == 1; 
}

void Remote_BlockingWaitData(void) {
	IBus_Control_Data.valid = 0; 
	Promise_TypeDef promise; 
	Promise_Set(&promise, Remote_PromiseCallback_DataReady); 
	await(&promise); 
}

void Remote_ProcessData(void) {
	/* Channel Assignments: 
			CH0: Direction (Left / Right)
			CH1: Speed (minor, < 1025 makes backward)
			CH2: Throttle
			CH3: Distribution (within one side)
			CH4: Takeover (> 1500)
			CH5: Power Src (1000, 1500, 2000)
	*/
	if(IBus_Control_Data.channel[4] < 1500) {
		if(Remote_State.TakenOver) {
			Motor_Sync_Takeover = 0; 
			Remote_State.TakenOver = 0; 
			Remote_State.MotorState = 0; 
			
			Motor_Power_Control.Fault = MOTOR_FAULT_NOFAULT; 
			
			for(int i = 0; i < 4; i++) {
				Motor_Channel_Controls[i].Mode = MOTOR_MODE_STOP; 
				Motor_Channel_Controls[i].PWM = 0; 
			}
		}
		return; 
	}
	if(!Remote_State.TakenOver) {
		Motor_Sync_Takeover = 1; 
		Remote_State.TakenOver = 1; 
		Remote_State.PowerScheme = -1; 
		Remote_State.MotorState = 0; 
		
		Motor_Power_Control.ForceRun = 0; 
		if(IBus_Control_Data.channel[0] - 1000 < 25) 
			if(IBus_Control_Data.channel[1] - 1000 < 25) 
				Motor_Power_Control.ForceRun = 1; 
		Motor_Power_Control.Fault = MOTOR_FAULT_NOFAULT; 
		
		for(int i = 0; i < 4; i++) {
			Motor_Channel_Controls[i].Mode = MOTOR_MODE_BRAKE; 
			Motor_Channel_Controls[i].PWM = 4096; 
		}
	}
	
	int power_scheme; 
	if(IBus_Control_Data.channel[5] < 1250) 
		power_scheme = 1; 
	else if(IBus_Control_Data.channel[5] > 1750) 
		power_scheme = 2; 
	else 
		power_scheme = 0; 
	if(power_scheme != Remote_State.PowerScheme) {
		switch(power_scheme) {
			default: 
				Motor_Power_Control.Power = 0x0C; 
				break; 
			case 1: 
				Motor_Power_Control.Power = 0x3D; 
				break; 
			case 2: 
				Motor_Power_Control.Power = 0x3F; 
				break; 
		}
		Remote_State.PowerScheme = power_scheme; 
	}
	
	int throttle = IBus_Control_Data.channel[2] - 1000; 
	if(throttle > 1000) throttle = 1000; 
	if(throttle < 0) throttle = 0; 
	
	if(throttle < 100) {
		if(!Remote_State.MotorState) return; // Already stopped
		Remote_State.MotorState = 0; 
		for(int i = 0; i < 4; i++) 
			Motor_Channel_Controls[i].Mode = MOTOR_MODE_BRAKE; 
		throttle = 0; 
	}
	else {
		if(!Remote_State.MotorState) {
			Remote_State.MotorState = 1; 
			int mode = MOTOR_MODE_FORWARD; 
			if(IBus_Control_Data.channel[1] - 1000 < 25) 
				mode = MOTOR_MODE_BACKWARD; 
			for(int i = 0; i < 4; i++) 
				Motor_Channel_Controls[i].Mode = mode; 
		}
		throttle = (throttle - 100) / 9 * 10; // Scale 0~1000
	}
	
	int speed = IBus_Control_Data.channel[1] - 1500; 
	if(speed > 500) speed = 500; 
	if(speed < -500) speed = -500; 
	
	throttle = throttle * (2000 + speed) / 2000; // Scale 25%
	if(throttle > 1000) throttle = 1000; 
	if(throttle < 0) throttle = 0; 
	
	int direction = IBus_Control_Data.channel[0] - 1500; 
	if(direction > 500) direction = 500; 
	if(direction < -500) direction = -500; 
	
	int group_left = throttle * (500 + direction) / 500; // Scale 100%
	int group_right = throttle * (500 - direction) / 500; // Scale 100% 
	
	if(group_left > 1000) group_left = 1000; 
	if(group_left < 0) group_left = 0; 
	if(group_right > 1000) group_right = 1000; 
	if(group_right < 0) group_right = 0; 
	
	int distribution = IBus_Control_Data.channel[3] - 1500; 
	if(distribution > 500) distribution = 500; 
	if(distribution < -500) distribution = -500; 
	
	int motor_LA = group_left * (2000 - distribution) / 2000; // Scale 25%
	int motor_LB = group_left * (2000 + distribution) / 2000; // Scale 25% 
	int motor_RA = group_right * (2000 + distribution) / 2000; // Scale 25%
	int motor_RB = group_right * (2000 - distribution) / 2000; // Scale 25% 
	
	if(motor_LA > 1000) motor_LA = 1000; 
	if(motor_LA < 0) motor_LA = 0; 
	if(motor_LB > 1000) motor_LB = 1000; 
	if(motor_LB < 0) motor_LB = 0; 
	if(motor_RA > 1000) motor_RA = 1000; 
	if(motor_RA < 0) motor_RA = 0; 
	if(motor_RB > 1000) motor_RB = 1000; 
	if(motor_RB < 0) motor_RB = 0; 
	
	motor_LA = motor_LA * 4095 / 1000; 
	motor_LB = motor_LB * 4095 / 1000; 
	motor_RA = motor_RA * 4095 / 1000; 
	motor_RB = motor_RB * 4095 / 1000; 
	
	Motor_Channel_Controls[0].DesiredVoltage = Motor_CalcVoPercent(motor_LA); 
	Motor_Channel_Controls[1].DesiredVoltage = Motor_CalcVoPercent(motor_LB); 
	Motor_Channel_Controls[2].DesiredVoltage = Motor_CalcVoPercent(motor_RA); 
	Motor_Channel_Controls[3].DesiredVoltage = Motor_CalcVoPercent(motor_RB); 
	
	return; 
}

void Remote_Task_Func(void) {
	Remote_Telemetry_Ready = 0; 
	Remote_State.PowerScheme = -1; 
	Remote_State.TakenOver = 0; 
	IBus_Init(); 
	for(int i = 0; i < IBUS_SENSOR_COUNT; i++) 
		IBus_Sensor_Type[i] = Motor_Telemetry_Sensor_Type[i]; 
	Remote_Telemetry_Ready = 1; 
	
	for(;;) {
		Remote_BlockingWaitData(); 
		Remote_ProcessData(); 
	}
}
