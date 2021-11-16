#include <stm32f10x.h>
#include <Variables.h>
#include "Task.h"
#include "GPIO.h"
#include "Buzzer.h"
#include "SoftI2C.h"

#include "Motor.h"
#include "ExtSensor.h"
#include "Session.h"
#include "Remote.h"
#include "UI.h"

Task_TypeDef Motor_Task; 
Task_TypeDef ExtSensor_Task; 
Task_TypeDef Session_Task; 
Task_TypeDef Remote_Task; 
Task_TypeDef UI_Task; 

int main(void) {
	unsigned int * ptr = ALLOC_ROM_END; 
	for(; ptr < RAM_END; ptr++) 
		*ptr = 0xDEADBEEF;  
	
	TaskSystem_Init(); 
	GPIO_Init(); 
	I2C_Init(); 
	Buzzer_Init(); 
	
	Task_Init(&Motor_Task, TASK_STACK_MOTOR, Motor_Task_Func); 
	Task_Init(&ExtSensor_Task, TASK_STACK_EXTSENSOR, ExtSensor_Task_Func); 
	Task_Init(&Session_Task, TASK_STACK_SESSION, Session_Task_Func); 
	Task_Init(&Remote_Task, TASK_STACK_REMOTE, Remote_Task_Func); 
	Task_Init(&UI_Task, TASK_STACK_UI, UI_Task_Func); 
	
	while(1) {
		Task_Schedule(&Motor_Task); 
		Task_Schedule(&Session_Task); 
		if(!Motor_Task_Busy) // Brute scheduler resolving I2C Traffic Jitter
			Task_Schedule(&ExtSensor_Task); 
		Task_Schedule(&Remote_Task); 
		Task_Schedule(&UI_Task); 
	}
}

