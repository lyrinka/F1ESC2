#ifndef __TACHO_H__
#define __TACHO_H__

typedef struct {
	unsigned int cycle; 
	unsigned short subcycle; 
	short average; // Q1.12
	int interval; 
	int rpm; 
} Tacho_Channels_TypeDef; 

extern volatile unsigned int Tacho_CurrentCycle; 
extern Tacho_Channels_TypeDef Tacho_Channels[4]; 

extern void Tacho_Init(void); 
void Tacho_SetCallback(void (*)(void));

#endif
