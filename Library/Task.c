#include <stm32f10x.h>
#include <Variables.h>
#include "Task.h"

Task_TypeDef * Task_Running; 

void TaskSystem_Init(void) {
	NVIC_SetPriorityGrouping(PRIORITY_GROUPING); 
	NVIC_SetPriority(SVCall_IRQn, PRIORITY_SVC); 
	Task_Running = 0; 
}

void Task_Init(Task_TypeDef * task, unsigned int * stack, void * func) {
	stack -= 16; 
	stack[15] = 0x01000000; 
	stack[14] = (unsigned int)func; 
	task->promise = 0; 
	task->stack = stack; 
}

__svc(0x0) void Task_Yield(void); 
__svc(0x0) unsigned int * Task_Call(unsigned int *); 

void Task_Schedule(Task_TypeDef * task) {
	int resolved = 0; 
	Promise_TypeDef * promise = task->promise; 
	if(!promise) 
		resolved = 1; 
	else {
		if(!promise->resolved) 
			promise->resolved = promise->condition(); 
		if(promise->resolved) 
			resolved = 1; 
	}
	if(resolved) {
		Task_Running = task; 
		task->stack = Task_Call(task->stack); 
	}
}

void Task_Await(Promise_TypeDef * promise) {
	Task_Running->promise = promise; 
	Task_Yield(); 
}

extern void Promise_Set(Promise_TypeDef * promise, int (*condition)(void)) {
	promise->condition = condition; 
	promise->resolved = 0; 
}

__asm void SVC_Handler(void) {
		TST		LR, #4
		BNE		SwPSP
SwMSP
		LDR		R0, [SP]
		PUSH	{R4-R11}
	
		LDMIA	R0!, {R4-R11}
		MSR		PSP, R0
		MOVS	LR, #0xFFFFFFFD
		BX		LR
SwPSP
		MRS		R0, PSP
		STMDB	R0!, {R4-R11}
		POP		{R4-R11}
		STR		R0, [SP]
		MOVS	LR, #0xFFFFFFF9
		BX		LR
}
