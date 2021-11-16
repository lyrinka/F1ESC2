#ifndef __EXTSENSOR_H__
#define __EXTSENSOR_H__

extern short ExtSensor_Motor_Temp[4]; 
extern short ExtSensor_Battery_Temp; 
extern short ExtSensor_Ambient_Temp; 

extern void ExtSensor_Task_Func(void); 

extern void ExtSensor_SetUpdateCallback(void (*)(void)); 
extern void ExtSensor_SetTiltCallback(void (*)(void)); 

#endif
