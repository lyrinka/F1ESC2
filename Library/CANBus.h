#ifndef __CANBUS_H__
#define __CANBUS_H__

typedef struct {
	unsigned int id; 
	int len; 
	unsigned char data[8]; 
} CAN_Packet_TypeDef; 

extern void CAN_Init(void); 

extern int CAN_BlockingTransmit(CAN_Packet_TypeDef *); 
extern int CAN_BlockingReceive(CAN_Packet_TypeDef *); 

#endif
