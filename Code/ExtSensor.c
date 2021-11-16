#include <stm32f10x.h>
#include <Variables.h>
#include <Task.h>
#include "ExtSensor.h"

#include "Tacho.h" // For delaying

#include "DS2482.h"
#include "DS18B20.h"
#include "LIS3DH.h"

#define DS2482 0x30
#define LIS3DH 0x32

const DS18B20_ID_TypeDef DS18B20_Motor_IDs[4] = {
	{0x28, 0x68, 0x5F, 0x95, 0xF0, 0x01, 0x3C, 0xB2}, 
	{0x28, 0x65, 0x6D, 0x95, 0xF0, 0x01, 0x3C, 0xFF}, 
	{0x28, 0x8E, 0xDC, 0x96, 0xF0, 0x01, 0x3C, 0x24}, 
	{0x28, 0x27, 0xA7, 0x96, 0xF0, 0x01, 0x3C, 0xF3}, 
}; 

const DS18B20_ID_TypeDef DS18B20_Battery_ID = {
	 0x28, 0xB5, 0xA9, 0x95, 0xF0, 0x01, 0x3C, 0x06, 
}; 

short ExtSensor_Motor_Temp[4]; 
short ExtSensor_Battery_Temp; 
short ExtSensor_Ambient_Temp; 

void (*ExtSensor_UpdateCallback)(void); 
void (*ExtSensor_TiltCallback)(void); 

// Core Logic
int ExtSensor_DelayTarget; 

int ExtSensor_PromiseCallback_Delay(void) {
	return Tacho_CurrentCycle > ExtSensor_DelayTarget; 
}

void ExtSensor_Delay(int cycles) { // Caution! This delay functions links with tachometer
	ExtSensor_DelayTarget = Tacho_CurrentCycle + cycles; 
	Promise_TypeDef promise; 
	promise.condition = ExtSensor_PromiseCallback_Delay; 
	promise.resolved = 0; 
	await(&promise); 
}

void ExtSensor_SetUpdateCallback(void (*updatecallback)(void)) {
	ExtSensor_UpdateCallback = updatecallback; 
}

void ExtSensor_SetTiltCallback(void (*tiltcallback)(void)) {
	ExtSensor_TiltCallback = tiltcallback; 
}

void ExtSensor_Task_Func(void) {
	DS2482_Init(DS2482); 
	ExtSensor_Delay(1); 
	LIS3DH_Init(LIS3DH); 
	ExtSensor_Delay(5); 
	
	AFIO->EXTICR[0] = AFIO->EXTICR[0] & 0x0FFF | 0x1000; 
	EXTI->IMR 	|=  0x0008; 
	EXTI->EMR 	&= ~0x0008; 
	EXTI->RTSR	|=  0x0008; 
	EXTI->FTSR 	&= ~0x0008; 
	EXTI->PR		 =  0x0008; 
	
	NVIC_SetPriority(EXTI3_IRQn, PRIORITY_EXTI3); 
	NVIC_ClearPendingIRQ(EXTI3_IRQn); 
	NVIC_EnableIRQ(EXTI3_IRQn); 
	
	for(;;) {
		DS18B20_Measurement_TypeDef meas; 
		DS18B20_ConvertAllAsync(DS2482); 
		ExtSensor_Delay(10); 
		for(int i = 0; i < 4; i++) {
			if(DS18B20_ReadMeasurement(DS2482, &DS18B20_Motor_IDs[i], &meas)) 
				meas.temp = 0x8000; 
			ExtSensor_Motor_Temp[i] = meas.temp; 
		}
		if(DS18B20_ReadMeasurement(DS2482, &DS18B20_Battery_ID, &meas)) 
			meas.temp = 0x8000; 
		ExtSensor_Battery_Temp = meas.temp; 
		ExtSensor_Ambient_Temp = LIS3DH_Read_Temperature(LIS3DH); 
		if(ExtSensor_UpdateCallback) ExtSensor_UpdateCallback(); 
	}
}

void EXTI3_IRQHandler(void) {
	EXTI->PR = 0x0008; 
	ExtSensor_TiltCallback(); 
}
