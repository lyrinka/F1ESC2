#include <stm32f10x.h>
#include <Task.h>
#include <GPIO.h>
#include "Motor.h"

#include "PCF8574.h"
#include "MCP4728.h"
#include "INA3221.h"
#include "ADS7830.h"

#include "Buzzer.h"
#include "Audio.h"
#include "PWM.h"
#include "Tacho.h"
#include "ExtSensor.h"
#include "Remote.h"

#define PCF	 0x40
#define MCP	 0xC4
#define INA1 0x80
#define INA2 0x82
#define ADS  0x90

// Core Logic
#define CONTROL_VOLTAGE_MAX 4095
#define CONTROL_VOLTAGE_MIN 0
#define CONTROL_VOLTAGE_IDLE CONTROL_VOLTAGE_MAX

#define SETPOINT_VOLTAGE_MAX 750
#define SETPOINT_VOLTAGE_MIN 250 // 2V
#define SETPOINT_VOLTAGE_INITIAL SETPOINT_VOLTAGE_MIN

#define PERCENTAGE_FULLSCALE 4095

#define VPID_KI 192
#define VPID_LOCKTIME 10
#define VPID_TOLERANCE 13

#define MINIMUM_VOLTAGE 800 // 6.4V
#define MAXIMUM_POWER_CURRENT 3750 // 3A
#define MAXIMUM_MOTOR_CURRENT 2500 // 2A
#define MAXIMUM_BOARD_TEMP 70 // 69.8C
#define MAXIMUM_BATTERT_TEMP 720 // 45.0C
#define MAXIMUM_MOTOR_TEMP 960 // 60.0C
#define MAXIMUM_STUCK_TIME 25 // 0.5s
#define STUCK_CURRENT 60 // 48mA
#define MAXIMUM_SYNC_TIME 500 // 10s

volatile int Motor_Task_Initialized; 
volatile int Motor_Task_Busy; 
volatile int Motor_Loop_Late; 
volatile int Motor_Sync_Received; 
volatile int Motor_Sync_Takeover; 

Motor_VPID_Controller_TypeDef Motor_VPID_Controller; 

Motor_Channel_Measurement_TypeDef 		Motor_Channel_Measurements[4]; 
Motor_Channel_Control_TypeDef 				Motor_Channel_Controls[4]; 
Motor_Power_Measurement_TypeDef 			Motor_Power_Measurement; 
Motor_Power_Control_TypeDef 					Motor_Power_Control; 
Motor_Thermistor_Measurement_TypeDef 	Motor_Thermistor_Measurement; 

Motor_History_Data_TypeDef 						Motor_History_Data; 

short Motor_CalcVoPercent(short percent) { // Caution! This is an external function and may be called asynchronously. 
	if(percent > PERCENTAGE_FULLSCALE) percent = PERCENTAGE_FULLSCALE; 
	if(percent < 0) percent = 0; 
	return (int)percent * (SETPOINT_VOLTAGE_MAX - SETPOINT_VOLTAGE_MIN) / PERCENTAGE_FULLSCALE + SETPOINT_VOLTAGE_MIN; 
}

void Motor_TachoUpdateCallback(void) { // Caution! This function executes in IRQ context.
	for(int i = 0; i < 4; i++) 
		Motor_Channel_Measurements[i].Speed = Tacho_Channels[i].rpm; 
}

void Motor_ExtSensor_UpdateCallback(void) { // Caution! This function executes in ExtTemp task context. 
	for(int i = 0; i < 4; i++) 
		Motor_Channel_Measurements[i].Temperature = ExtSensor_Motor_Temp[i]; 
	Motor_Power_Measurement.BattTemperature = ExtSensor_Battery_Temp; 
	Motor_Power_Measurement.AmbientTemperature = ExtSensor_Ambient_Temp; 
}

void Motor_ExtSensor_TiltCallback(void) { // Caution! This function executes in IRQ context.
	Motor_Power_Control.Fault |= MOTOR_FAULT_TILT; 
}

void Motor_EmergencyStop(void) { // Caution! This is an external function and may be called asynchronously or in interrupts. 
	PWM_Disable(); // First thing is to set MOE = 0. 
	GPIO_SW2_DIS(); // Next we plug off the master switch. 
	Motor_Power_Control.Power &= ~(MOTOR_POWER_MASTER_SWITCH | MOTOR_POWER_MOTOR_DRIVE | MOTOR_POWER_VPID_LOOP); // We turn everything off. 
	Motor_Power_Control.Fault |= MOTOR_FAULT_BRAKE; // Set a fault flag
	for(int i = 0; i < 4; i++) { // Finally We reset motors
		Motor_Channel_Controls[i].Mode = MOTOR_MODE_STOP; 
		Motor_Channel_Controls[i].PWM = 0; 
		Motor_Channel_Controls[i].DesiredVoltage = SETPOINT_VOLTAGE_MIN; 
		Motor_Channel_Controls[i].ControlVoltage = CONTROL_VOLTAGE_MAX; 
	}
}

void Motor_Buzz(void) { // This function is introduced to make noise (and temporization delay)
	GPIO_LED_LINK_ON(); 
	
	Audio_Buffer_Shadow(Audio_Motor_Beep_Length, Audio_Motor_Beep_Note, Audio_Motor_Buffer); 
	Buzzer_Play(Audio_Motor_Beep_Length, Audio_Motor_Buffer, Audio_Motor_Beep_Time); 
	
	GPIO_LED_LINK_OFF(); 
}

void Motor_Task_Initialize(void) {
	// Initialization of data structures
	Motor_Task_Busy = 0; 
	Motor_Loop_Late = 0; 
	Motor_Sync_Received = 0; 
	Motor_Sync_Takeover = 0; 
	Motor_VPID_Controller.KI = VPID_KI; 
	Motor_VPID_Controller.LockTime = VPID_LOCKTIME; 
	Motor_VPID_Controller.Tolerance = VPID_TOLERANCE; 
	for(int i = 0; i < 4; i++) {
		Motor_Channel_Controls[i].ControlVoltage = CONTROL_VOLTAGE_IDLE; 
		Motor_Channel_Controls[i].DesiredVoltage = Motor_CalcVoPercent(0); 
		Motor_Channel_Controls[i].Mode = MOTOR_MODE_STOP; 
		Motor_Channel_Controls[i].PWM = 0; 
		
		Motor_History_Data.ControlVoltage[i] = ~CONTROL_VOLTAGE_IDLE; 
		Motor_History_Data.PWM[i] = 0; 
	}
	Motor_Power_Control.MinVoltage = MINIMUM_VOLTAGE; 
	Motor_Power_Control.MaxPowerCurrent = MAXIMUM_POWER_CURRENT; 
	Motor_Power_Control.MaxMotorCurrent = MAXIMUM_MOTOR_CURRENT; 
	Motor_Power_Control.MaxBoardTemp = MAXIMUM_BOARD_TEMP; 
	Motor_Power_Control.MaxBatteryTemp = MAXIMUM_BATTERT_TEMP; 
	Motor_Power_Control.MaxMotorTemp = MAXIMUM_MOTOR_TEMP; 
	Motor_Power_Control.StuckCurrent = STUCK_CURRENT; 
	Motor_Power_Control.StuckTime = MAXIMUM_STUCK_TIME; 
	for(int i = 0; i < 4; i++) 
		Motor_Power_Control.StuckCounter[i] = 0; 
	Motor_Power_Control.SyncTime = MAXIMUM_SYNC_TIME; 
	Motor_Power_Control.SyncCounter = 0; 
	Motor_Power_Control.Fault = 0x00; 
	Motor_Power_Control.Power = 0x00; 
	Motor_Power_Control.ForceRun = 0; 
	Motor_History_Data.Power_Bitmap = 0x00; 
	Motor_History_Data.Power_InFault = 0; 
	Motor_Thermistor_Measurement.Valid = 0; 

	// Initialization of GPIO ports
	GPIO_SW1_DIS(); 
	GPIO_SW2_DIS(); 
	GPIO_SPD_DIS(); 
	GPIO_TEMP_DIS(); 
	
	// Initialization of I2C slaves
	// IO
	PCF8574_Output(PCF, 0xFF); 
	// DAC
	for(int i = 0; i < 4; i++) 
		MCP4728_SetChannelHigh(MCP, i); 
	// PWR
	INA3221_Init(INA1); 
	INA3221_Init(INA2); 
	// ADC
	ADS7830_Read(ADS, ADS7830_CH0); // That's a dummy read
	
	// Initialization of External Components
	// PWM
	PWM_Init(); 
	// Tachometer
	Tacho_Init(); 
	
	// Temporization
	Motor_Buzz(); 
	
	// Set Callbacks
	Tacho_SetCallback(Motor_TachoUpdateCallback); 
	ExtSensor_SetUpdateCallback(Motor_ExtSensor_UpdateCallback); 
	ExtSensor_SetTiltCallback(Motor_ExtSensor_TiltCallback); 
}

void Motor_Task_Update_Measurement(void) {
	typedef struct {
		short Current; 
		short Voltage; 
	} Motor_Channel_Measurement_Partial_TypeDef; 
	typedef struct {
		short UPSVoltage; 
		short UPSCurrent; 
		short PWRVoltage; 
		short PWRCurrent; 
	} Motor_Power_Measurement_Partial_TypeDef; 
	
	Motor_Channel_Measurement_Partial_TypeDef Motor_Channel_Measurements_shadow[4]; 
	Motor_Power_Measurement_Partial_TypeDef 	Motor_Power_Measurement_shadow; 
	Motor_Thermistor_Measurement_TypeDef 			Motor_Thermistor_Measurement_shadow; 
	INA3221_Measurement_TypeDef INA3221_Measurement; 
	
	int thermistor_enable = Motor_Power_Control.Power & MOTOR_POWER_THERMISTOR; 
	
	// Data Acquisition
	INA3221_Read_Measurement(INA1, &INA3221_Measurement); 
	Motor_Power_Measurement_shadow.PWRCurrent = INA3221_Measurement.CHxA[0]; 
	Motor_Power_Measurement_shadow.PWRVoltage = INA3221_Measurement.CHxV[0]; 
	Motor_Channel_Measurements_shadow[0].Current = INA3221_Measurement.CHxA[1]; 
	Motor_Channel_Measurements_shadow[0].Voltage = INA3221_Measurement.CHxV[1]; 
	Motor_Channel_Measurements_shadow[1].Current = INA3221_Measurement.CHxA[2]; 
	Motor_Channel_Measurements_shadow[1].Voltage = INA3221_Measurement.CHxV[2]; 
	
	INA3221_Read_Measurement(INA2, &INA3221_Measurement); 
	Motor_Power_Measurement_shadow.UPSCurrent = INA3221_Measurement.CHxA[0]; 
	Motor_Power_Measurement_shadow.UPSVoltage = INA3221_Measurement.CHxV[0]; 
	Motor_Channel_Measurements_shadow[2].Current = INA3221_Measurement.CHxA[1]; 
	Motor_Channel_Measurements_shadow[2].Voltage = INA3221_Measurement.CHxV[1]; 
	Motor_Channel_Measurements_shadow[3].Current = INA3221_Measurement.CHxA[2]; 
	Motor_Channel_Measurements_shadow[3].Voltage = INA3221_Measurement.CHxV[2]; 
	
	if(thermistor_enable) {
		Motor_Thermistor_Measurement_shadow.Converter[0] = ADS7830_Read(ADS, ADS7830_CH0); 
		Motor_Thermistor_Measurement_shadow.Converter[1] = ADS7830_Read(ADS, ADS7830_CH2); 
		Motor_Thermistor_Measurement_shadow.Converter[2] = ADS7830_Read(ADS, ADS7830_CH4); 
		Motor_Thermistor_Measurement_shadow.Converter[3] = ADS7830_Read(ADS, ADS7830_CH6); 
		Motor_Thermistor_Measurement_shadow.Switch[0] = ADS7830_Read(ADS, ADS7830_CH1); 
		Motor_Thermistor_Measurement_shadow.Switch[1] = ADS7830_Read(ADS, ADS7830_CH3); 
		Motor_Thermistor_Measurement_shadow.Switch[2] = ADS7830_Read(ADS, ADS7830_CH5); 
		Motor_Thermistor_Measurement_shadow.Switch[3] = ADS7830_Read(ADS, ADS7830_CH7); 
	}
	
	// Data shadowing
	for(int i = 0; i < 4; i++) {
		Motor_Channel_Measurements[i].Current = Motor_Channel_Measurements_shadow[i].Current; 
		Motor_Channel_Measurements[i].Voltage = Motor_Channel_Measurements_shadow[i].Voltage; 
	}
	Motor_Power_Measurement.UPSVoltage = Motor_Power_Measurement_shadow.UPSVoltage; 
	Motor_Power_Measurement.UPSCurrent = Motor_Power_Measurement_shadow.UPSCurrent; 
	Motor_Power_Measurement.PWRVoltage = Motor_Power_Measurement_shadow.PWRVoltage; 
	Motor_Power_Measurement.PWRCurrent = Motor_Power_Measurement_shadow.PWRCurrent; 
	if(thermistor_enable) {
		Motor_Thermistor_Measurement_shadow.Valid = 1; 
		Motor_Thermistor_Measurement = Motor_Thermistor_Measurement_shadow; 
	}
	else Motor_Thermistor_Measurement.Valid = 0; 
	
	// Fault management
	if(!Motor_Sync_Takeover) {
		if(!(Motor_Power_Control.Fault & MOTOR_FAULT_LOSESYNC)) {
			if(!Motor_Sync_Received) {
				if(Motor_Power_Control.SyncCounter > Motor_Power_Control.SyncTime) {
					Motor_Power_Control.Fault |= MOTOR_FAULT_LOSESYNC; 
					Motor_Power_Control.SyncCounter = 0; 
				}
				else Motor_Power_Control.SyncCounter++; 
			}
			else {
				Motor_Sync_Received = 0; 
				Motor_Power_Control.SyncCounter = 0; 
			}
		}
	}
	else Motor_Power_Control.SyncCounter = 0; 
	
	if(Motor_Power_Measurement.PWRVoltage < Motor_Power_Control.MinVoltage) 
		Motor_Power_Control.Fault |= MOTOR_FAULT_UNDERVOLTAGE; 
	if(Motor_Power_Measurement.PWRCurrent > Motor_Power_Control.MaxPowerCurrent) 
		Motor_Power_Control.Fault |= MOTOR_FAULT_OVERCURRENT; 
	if(Motor_Power_Measurement.BattTemperature > Motor_Power_Control.MaxBatteryTemp) 
		Motor_Power_Control.Fault |= MOTOR_FAULT_HOTBATTERY; 
	for(int i = 0; i < 4; i++) {
		if(Motor_Channel_Measurements[i].Current > Motor_Power_Control.MaxMotorCurrent) 
			Motor_Power_Control.Fault |= MOTOR_FAULT_OVERCURRENT; 
		if(Motor_Channel_Measurements[i].Temperature > Motor_Power_Control.MaxMotorTemp) 
			Motor_Power_Control.Fault |= MOTOR_FAULT_OVERTEMP; 
		
		if(Motor_Power_Control.Power & MOTOR_POWER_SPEED_SENSOR) {
			if((Motor_Channel_Measurements[i].Current > Motor_Power_Control.StuckCurrent) && (Motor_Channel_Measurements[i].Speed < 0)) {
				if(Motor_Power_Control.StuckCounter[i] > Motor_Power_Control.StuckTime) 
					Motor_Power_Control.Fault |= MOTOR_FAULT_STUCK; 
				else Motor_Power_Control.StuckCounter[i]++; 
			}
			else Motor_Power_Control.StuckCounter[i] = 0; 
		}
		else Motor_Power_Control.StuckCounter[i] = 0; 
		
		if(!Motor_Thermistor_Measurement.Valid) continue; 
		if(Motor_Thermistor_Measurement.Converter[i] < Motor_Power_Control.MaxBoardTemp) 
			Motor_Power_Control.Fault |= MOTOR_FAULT_OVERTEMP; 
		if(Motor_Thermistor_Measurement.Switch[i] < Motor_Power_Control.MaxBoardTemp) 
			Motor_Power_Control.Fault |= MOTOR_FAULT_OVERTEMP; 
	}
}

void Motor_Task_Apply_Control(void) {
	Motor_Channel_Control_TypeDef 				Motor_Channel_Controls_shadow[4]; 
	// Data shadowing
	for(int i = 0; i < 4; i++) 
		Motor_Channel_Controls_shadow[i] = Motor_Channel_Controls[i]; 
	// Fault management
	if(Motor_Power_Control.Fault && !Motor_Power_Control.ForceRun) {
		if(Motor_History_Data.Power_InFault && (Motor_Power_Control.Fault & MOTOR_FAULT_BRAKE)) // Already done necessary operations, lockup
			return; 
		Motor_EmergencyStop(); 
		Motor_History_Data.Power_InFault = 1; 
	}
	else 
		Motor_History_Data.Power_InFault = 0; 
	// Data Update
	// Switches & MOSFETs
	if(Motor_Power_Control.Power != Motor_History_Data.Power_Bitmap) {
		if(Motor_Power_Control.Power & MOTOR_POWER_MASTER_SWITCH) 
			GPIO_SW2_EN(); 
		else 
			GPIO_SW2_DIS(); 
		if(Motor_Power_Control.Power & MOTOR_POWER_BACKUP_SWITCH) 
			GPIO_SW1_EN(); 
		else
			GPIO_SW1_DIS(); 
		if(Motor_Power_Control.Power & MOTOR_POWER_SPEED_SENSOR) 
			GPIO_SPD_EN(); 
		else
			GPIO_SPD_DIS(); 
		if(Motor_Power_Control.Power & MOTOR_POWER_THERMISTOR) 
			GPIO_TEMP_EN(); 
		else
			GPIO_TEMP_DIS(); 
		if(Motor_Power_Control.Power & MOTOR_POWER_MOTOR_DRIVE) 
			PWM_Enable(); 
		else 
			PWM_Disable(); 
		Motor_History_Data.Power_Bitmap = Motor_Power_Control.Power; 
	}
	// Mode
	unsigned char PCF8574_Data = 0; 
	for(int i = 0; i < 4; i++) 
		PCF8574_Data |= (Motor_Channel_Controls_shadow[i].Mode & 0x3) << (i << 1); 
	PCF8574_Data = ~PCF8574_Data; 
	if(PCF8574_Data != Motor_History_Data.PCF_Data) {
		PCF8574_Output(PCF, PCF8574_Data); 
		Motor_History_Data.PCF_Data = PCF8574_Data; 
	}
	// PWM
	for(int i = 0; i < 4; i++) {
		unsigned short temp = Motor_Channel_Controls_shadow[i].PWM; 
		if(temp == Motor_History_Data.PWM[i]) continue; 
		PWM_Set(i, temp); 
		Motor_History_Data.PWM[i] = temp; 
	}
	// Voltage Control
	for(int i = 0; i < 4; i++) {
		short temp = Motor_Channel_Controls_shadow[i].ControlVoltage; 
		if(temp == Motor_History_Data.ControlVoltage[i]) continue; 
		if(temp > CONTROL_VOLTAGE_MAX) temp = CONTROL_VOLTAGE_MAX; 
		if(temp < CONTROL_VOLTAGE_MIN) temp = CONTROL_VOLTAGE_MIN; 
		MCP4728_SetChannelVoltage(MCP, i, temp); 
		Motor_History_Data.ControlVoltage[i] = temp; 
	}
}

void Motor_Task_VPID(void) {
	int KI = Motor_VPID_Controller.KI; 
	int Tolerance = Motor_VPID_Controller.Tolerance; 
	int LockTime = Motor_VPID_Controller.LockTime; 
	if(!(Motor_Power_Control.Power & MOTOR_POWER_VPID_LOOP)) {
		for(int i = 0; i < 4; i++) 
			Motor_Channel_Controls[i].ControlVoltage = CONTROL_VOLTAGE_IDLE; 
		return; 
	}
	for(int i = 0; i < 4; i++) {
		int setpoint = Motor_Channel_Controls[i].DesiredVoltage; 
		int error = Motor_Channel_Measurements[i].Voltage - setpoint; 
		int control = Motor_Channel_Controls[i].ControlVoltage + ((error * KI) >> 8); 
		if(control > CONTROL_VOLTAGE_MAX) control = CONTROL_VOLTAGE_MAX; 
		if(control < CONTROL_VOLTAGE_MIN) control = CONTROL_VOLTAGE_MIN; 
		Motor_Channel_Controls[i].ControlVoltage = control; 
		if(error < 0) error = -error; 
		error = (error << 8) / setpoint; 
		if(error > Tolerance) {
			Motor_VPID_Controller.LockCounter[i] = 0; 
			Motor_VPID_Controller.LockIndicator[i] = 0; 
		}
		else {
			if(Motor_VPID_Controller.LockCounter[i] < LockTime) {
				Motor_VPID_Controller.LockCounter[i]++; 
				Motor_VPID_Controller.LockIndicator[i] = 0; 
			}
			else {
				Motor_VPID_Controller.LockIndicator[i] = 1; 
			}
		}
	}
}

void Motor_Task_Func(void) {
	Motor_Task_Initialized = 0; 
	
	GPIO_LED_LINK_ON(); 
	Motor_Task_Initialize(); 
	GPIO_LED_LINK_OFF(); 
	
	int loopcounter = 0; 
	int looptarget; 
	int ledstate = 0; 
	
	Motor_Task_Initialized = 1; 
	
	// Data processing loop
	while(1) {
		if(PWM_BlockingWaitCycle()) 
			Motor_Loop_Late++; 
		
		if(Motor_Power_Control.Fault && !Motor_Power_Control.ForceRun) 
			looptarget = 4; 
		else if(Motor_Sync_Takeover)
			looptarget = 16; 
		else
			looptarget = 24; 
		if(++loopcounter > looptarget) {
			loopcounter = 0; 
			if(ledstate) 
				GPIO_LED_LINK_OFF(); 
			else 
				GPIO_LED_LINK_ON(); 
			ledstate = !ledstate; 
		}
		
		Motor_Task_Busy = 1; 
		Motor_Task_Update_Measurement(); 
		Motor_Task_Apply_Control(); 
		Motor_Task_Busy = 0; 
		Remote_Callback_Update_Telemetry(); 
		Motor_Task_VPID(); 
	}
}
