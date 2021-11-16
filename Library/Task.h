#ifndef __TASK_H__
#define __TASK_H__

typedef struct {
	int resolved; 
	int (*condition)(void); 
} Promise_TypeDef; 

typedef struct {
	unsigned int * stack; 
	Promise_TypeDef * promise; 
} Task_TypeDef; 

extern Task_TypeDef * Task_Running; 

extern void TaskSystem_Init(void); 
extern void Task_Init(Task_TypeDef *, unsigned int * stack, void * func); 

extern void Task_Schedule(Task_TypeDef *); 
extern void Task_Await(Promise_TypeDef *); 

#define await Task_Await

extern void Promise_Set(Promise_TypeDef *, int (*)(void)); 

#endif
