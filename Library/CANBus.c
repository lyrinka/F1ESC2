#include <stm32f10x.h>
#include <Task.h>
#include "CANBus.h"

void CAN_Init(void) {
	// CAN Initialization
	RCC->APB1ENR |= RCC_APB1ENR_CAN1EN; 
	CAN1->MCR = 0x8000; 
	CAN1->MCR = 0x0002; 
	CAN1->MCR = CAN1->MCR & (~0x02) | 0x01; // Init Request
	while(!(CAN1->MSR & 0x01)); 
	CAN1->BTR = 0x01230011; // 250kHz, ADuM3201 is f*cking slow
	CAN1->FMR = 0x01; 
	CAN1->FM1R = 0x00000000; 
	CAN1->FS1R = 0x00000001; 
	CAN1->FFA1R = 0x00000000; 
	CAN1->FA1R = 0x00000001; 
	CAN1->sFilterRegister[0] = (CAN_FilterRegister_TypeDef){0x00000000, 0x00000000}; 
	CAN1->FMR = 0x00; 
	CAN1->MCR = CAN1->MCR & (~0x01); 
	while(CAN1->MSR & 0x01); 
}

int CAN_PromiseCallback_WaitOnTransmit(void) {
	return !(CAN1->sTxMailBox[0].TIR & 0x1); 
}

int CAN_PromiseCallback_WaitOnReceive(void) {
	return (CAN1->RF0R & 0x03); 
}

int CAN_BlockingTransmit(CAN_Packet_TypeDef * packet) {
	Promise_TypeDef promise; 
	Promise_Set(&promise, CAN_PromiseCallback_WaitOnTransmit); 
	
	unsigned int * ptr = (unsigned int *)(&packet->data); 
	CAN1->sTxMailBox[0].TDLR = ptr[0]; 
	CAN1->sTxMailBox[0].TDHR = ptr[1]; 
	CAN1->sTxMailBox[0].TDTR = packet->len & 0x0F; 
	CAN1->sTxMailBox[0].TIR = 0x01; 
	
	await(&promise); 
	return 0; 
}

int CAN_BlockingReceive(CAN_Packet_TypeDef * packet) {
	Promise_TypeDef promise; 
	Promise_Set(&promise, CAN_PromiseCallback_WaitOnReceive); 
	await(&promise); 
	
	unsigned int * ptr = (unsigned int *)(&packet->data); 
	packet->id = CAN1->sFIFOMailBox[0].RIR >> 21; 
	packet->len = CAN1->sFIFOMailBox[0].RDTR & 0x0F; 
	ptr[0] = CAN1->sFIFOMailBox[0].RDLR; 
	ptr[1] = CAN1->sFIFOMailBox[0].RDHR; 
	CAN1->RF0R = 0x20; 
	return 0; 
}
