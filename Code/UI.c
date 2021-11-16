#include <stm32f10x.h>
#include <Task.h>
#include "UI.h"

#include "GPIO.h"
#include "Buzzer.h"
#include "Audio.h"
#include "Tacho.h"

#include "Motor.h"

// Core Logic
typedef struct {
	unsigned char Dirty; 
	unsigned char Fault; 
	unsigned char ForceRun; 
	unsigned char TakenOver; 
} UI_Fault_Record_TypeDef; 

UI_Fault_Record_TypeDef UI_Fault_Record; 

int UI_DelayTarget; 
int UI_ButtonState; 

int UI_PromiseCallback_Rendevous(void) {
	return Motor_Task_Initialized; 
}

void UI_Motor_Rendevous(void) {
	Promise_TypeDef promise; 
	Promise_Set(&promise, UI_PromiseCallback_Rendevous); 
	await(&promise); 
}

int UI_PromiseCallback_Delay(void) {
	return Tacho_CurrentCycle > UI_DelayTarget; 
}

void UI_Delay(int cycles) { // Caution! This delay functions links with tachometer
	UI_DelayTarget = Tacho_CurrentCycle + cycles; 
	Promise_TypeDef promise; 
	promise.condition = UI_PromiseCallback_Delay; 
	promise.resolved = 0; 
	await(&promise); 
}

void UI_Audio_Startup(void) {
	Audio_PlayNamed(UI, Startup); 
}
void UI_Audio_TakenOver_Enter(void) {
	Audio_PlayNamed(UI, TakenOver_Enter); 
}
void UI_Audio_TakenOver_Exit(void) {
	Audio_PlayNamed(UI, TakenOver_Exit); 
}
void UI_Audio_ForceRun_Enter(void) {
	Audio_PlayNamed(UI, ForceRun_Enter); 
}
void UI_Audio_ForceRun_Exit(void) {
	Audio_PlayNamed(UI, ForceRun_Exit); 
}
void UI_Audio_Fault_Enter(void) {
	Audio_PlayNamed(UI, Fault_Enter); 
}
void UI_Audio_Fault_Exit(void) {
	Audio_PlayNamed(UI, Fault_Exit); 
}
void UI_Audio_Fault_LoseSync_Enter(void) {
	Audio_PlayNamed(UI, Fault_LoseSync_Enter); 
}
void UI_Audio_Fault_Hot_Enter(void) {
	Audio_PlayNamed(UI, Fault_Hot_Enter); 
}
void UI_Audio_Fault_Power_Enter(void) {
	Audio_PlayNamed(UI, Fault_Power_Enter); 
}
void UI_Audio_Fault_Stuck_Enter(void) {
	Audio_PlayNamed(UI, Fault_Stuck_Enter); 
}
void UI_Audio_Fault_Tilt_Enter(void) {
	Audio_PlayNamed(UI, Fault_Tilt_Enter); 
}

void UI_Task_Func(void) {
	UI_ButtonState = -1; 
	UI_Fault_Record.Dirty = 1; 
	
	UI_Motor_Rendevous(); 
	
	UI_Audio_Startup(); 
	
	// Main loop
	for(;;) {
		unsigned char dirty = 0; 
		unsigned char fault = Motor_Power_Control.Fault; 
		unsigned char forcerun = Motor_Power_Control.ForceRun; 
		unsigned char takenover = Motor_Sync_Takeover; 
		if(UI_Fault_Record.Dirty) {
			dirty = 1; 
			UI_Fault_Record.Dirty = 0; 
		}
		if(!dirty) {
			if(fault != UI_Fault_Record.Fault || forcerun != UI_Fault_Record.ForceRun || takenover != UI_Fault_Record.TakenOver) 
				dirty = 1; 
		}
		if(dirty) {
			if(UI_Fault_Record.TakenOver != takenover) {
				if(takenover) 
					UI_Audio_TakenOver_Enter(); 
				else 
					UI_Audio_TakenOver_Exit(); 
			}
			
			if(UI_Fault_Record.ForceRun != forcerun) {
				if(forcerun) 
					UI_Audio_ForceRun_Enter(); 
				else 
					UI_Audio_ForceRun_Exit(); 
			}
			
			unsigned char difference = fault ^ UI_Fault_Record.Fault; 
			if(difference & MOTOR_FAULT_BRAKE) { // General Fault
				if(fault & MOTOR_FAULT_BRAKE) 
					UI_Audio_Fault_Enter(); 
				else 
					UI_Audio_Fault_Exit(); 
			}
			if(difference & MOTOR_FAULT_LOSESYNC) { // Lose Sync
				if(fault & MOTOR_FAULT_LOSESYNC) 
					UI_Audio_Fault_LoseSync_Enter(); 
			}
			if(difference & (MOTOR_FAULT_HOTBATTERY | MOTOR_FAULT_OVERTEMP)) { // Over Temperature
				if(fault & (MOTOR_FAULT_HOTBATTERY | MOTOR_FAULT_OVERTEMP)) 
					UI_Audio_Fault_Hot_Enter(); 
			}
			if(difference & (MOTOR_FAULT_OVERCURRENT | MOTOR_FAULT_UNDERVOLTAGE)) { // Voltage & Current
				if(fault & (MOTOR_FAULT_OVERCURRENT | MOTOR_FAULT_UNDERVOLTAGE)) 
					UI_Audio_Fault_Power_Enter(); 
			}
			if(difference & MOTOR_FAULT_STUCK) { // Motor Stuck
				if(fault & MOTOR_FAULT_STUCK) 
					UI_Audio_Fault_Stuck_Enter(); 
			}
			if(difference & MOTOR_FAULT_TILT) { // Device Tilt
				if(fault & MOTOR_FAULT_TILT) 
					UI_Audio_Fault_Tilt_Enter(); 
			}
			UI_Fault_Record.Fault = fault; 
			UI_Fault_Record.ForceRun = forcerun; 
			UI_Fault_Record.TakenOver = takenover; 
		}
		
		if(GPIO_KEY_READ()) 
			UI_ButtonState = 0; 
		else {
			if((UI_ButtonState >= 0) && (++UI_ButtonState > 5)) {
				Motor_Power_Control.Fault = MOTOR_FAULT_NOFAULT; 
				UI_ButtonState = -1; 
			}
		}
		
		UI_Delay(1); 
	}
}
