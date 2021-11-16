#ifndef __REMOTE_H__
#define __REMOTE_H__

typedef struct {
	int TakenOver; 
	int PowerScheme; 
	int MotorState; 
} Remote_State_TypeDef; 

extern void Remote_Task_Func(void); 

extern void Remote_Callback_Update_Telemetry(void); 

#endif
