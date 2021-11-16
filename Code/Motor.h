#ifndef __MOTOR_H__
#define __MOTOR_H__

#define MOTOR_MODE_STOP 0
#define MOTOR_MODE_FORWARD 1
#define MOTOR_MODE_BACKWARD 2
#define MOTOR_MODE_BRAKE 3

#define MOTOR_POWER_MASTER_SWITCH 0x01
#define MOTOR_POWER_BACKUP_SWITCH 0x02
#define MOTOR_POWER_SPEED_SENSOR 0x04
#define MOTOR_POWER_THERMISTOR 0x08
#define MOTOR_POWER_MOTOR_DRIVE 0x10
#define MOTOR_POWER_VPID_LOOP 0x20

#define MOTOR_FAULT_NOFAULT 0x00
#define MOTOR_FAULT_LOSESYNC 0x01
#define MOTOR_FAULT_HOTBATTERY 0x02
#define MOTOR_FAULT_OVERTEMP 0x04
#define MOTOR_FAULT_TILT 0x08
#define MOTOR_FAULT_BRAKE 0x10
#define MOTOR_FAULT_STUCK 0x20 
#define MOTOR_FAULT_UNDERVOLTAGE 0x40
#define MOTOR_FAULT_OVERCURRENT 0x80

typedef struct {
	short Voltage; // LSB 8mV
	short Current; // LSB 40uV, 0.8mA@0.05R
	short Speed; // RPM, -1 stop, EXTERNAL ACTOR UPDATE (IRQ)
	short Temperature; // LSB 1/16 degrees, signed, EXTERNAL ACTOR UPDATE (Task)
} Motor_Channel_Measurement_TypeDef; 

typedef struct {
	unsigned char Mode; // 0 stop, 1 foward, 2 backward, 3 brake
	unsigned short PWM; // 12-bit unsigned PWM, 0.1kHz
	short DesiredVoltage; // LSB 8mV, LATE UPDATE
	unsigned short ControlVoltage; // LSB 0.5mV, 12bit
} Motor_Channel_Control_TypeDef; 

typedef struct {
	short UPSVoltage; // LSB 8mV
	short UPSCurrent; // LSB 40uV, 0.8mA@0.05R
	short PWRVoltage; // LSB 8mV
	short PWRCurrent; // LSB 40uV, 0.8mA@0.05R
	short BattTemperature; // LSB 1/16 degrees, signed, EXTERNAL ACTOR UPDATE (Task)
	short AmbientTemperature; // LSB 1 degrees, signed, EXTERNAL ACTOR UPDATE (Task)
} Motor_Power_Measurement_TypeDef; 

typedef struct {
	unsigned char Power; // Power control bitmap
	unsigned char Fault; // Fault indicator bitmap
	unsigned char ForceRun; // Force Running
	unsigned char MaxBoardTemp; // Maximum On-board thermistor temperature
	short MinVoltage; // Minimum voltage for Power
	short MaxPowerCurrent; // Maximum sinking HV voltage source current
	short MaxMotorCurrent; // Maximum current for indivitual channels
	short MaxBatteryTemp; // Maximum Battery Temperature
	short MaxMotorTemp; // Maximum Motor Temperature
	short StuckCurrent; // Minimum Current Provided to trigger Stuck Fault
	int StuckTime; // Motor Stuck Time
	int StuckCounter[4]; // Motor Stuck Counter
	int SyncTime; 
	int SyncCounter; 
} Motor_Power_Control_TypeDef; 

typedef struct {
	unsigned char Valid; 
	unsigned char Converter[4]; 
	unsigned char Switch[4]; 
} Motor_Thermistor_Measurement_TypeDef; 

typedef struct {
	unsigned char PCF_Data; 
	unsigned char Power_Bitmap; 
	unsigned char Power_InFault; 
	unsigned short PWM[4]; 
	unsigned short ControlVoltage[4]; 
} Motor_History_Data_TypeDef; 

typedef struct {
	int KI; // KI in Q24.8
	int Tolerance; // Tolerance in Q24.8
	char LockTime; // LockTime in cycles
	char LockCounter[4]; 
	char LockIndicator[4]; // True for VPID Lockage, LATE UPDATE
} Motor_VPID_Controller_TypeDef; 

extern volatile int Motor_Task_Initialized; 
extern volatile int Motor_Task_Busy; 
extern volatile int Motor_Loop_Late; 
extern volatile int Motor_Sync_Received; 
extern volatile int Motor_Sync_Takeover; 

extern Motor_VPID_Controller_TypeDef Motor_VPID_Controller; 

extern Motor_Channel_Measurement_TypeDef 		Motor_Channel_Measurements[4]; 
extern Motor_Channel_Control_TypeDef 				Motor_Channel_Controls[4]; 
extern Motor_Power_Measurement_TypeDef 			Motor_Power_Measurement; 
extern Motor_Power_Control_TypeDef 					Motor_Power_Control; 
extern Motor_Thermistor_Measurement_TypeDef Motor_Thermistor_Measurement; 

extern void Motor_Task_Func(void); 

extern short Motor_CalcVoPercent(short); 
extern void Motor_EmergencyStop(void); 
extern void Motor_TiltFaultCallback(void); 

#endif
