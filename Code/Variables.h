#ifndef __VARIABLES_H__
#define __VARIABLES_H__

// Linker Stuff
#define ROM_START ((unsigned int *)0x08000000)
#define ROM_END   ((unsigned int *)0x08008000)
#define ROM_SIZE  0x8000 // 32kB
#define RAM_START ((unsigned int *)0x20000000)
#define RAM_END   ((unsigned int *)0x20002800)
#define RAM_SIZE  0x2800 // 10kB
extern unsigned int * const ALLOC_ROM_END; 

// Priority Stuff
#define PRIORITY_GROUPING 5
#define PRIORITY_TIM3     0
#define PRIORITY_TIM1_UP  1
#define PRIORITY_TIM2     2
#define PRIORITY_EXTI3    3
#define PRIORITY_SYSTICK  4
#define PRIORITY_USART2   8
#define PRIORITY_USART1   9
#define PRIORITY_SVC      15

// Task Stuff
#define TASK_STACK_MAIN				((unsigned int *)0x20002800)
#define TASK_STACK_MOTOR			((unsigned int *)0x20002400)
#define TASK_STACK_EXTSENSOR	((unsigned int *)0x20002000)
#define TASK_STACK_SESSION		((unsigned int *)0x20001C00)
#define TASK_STACK_REMOTE			((unsigned int *)0x20001800)
#define TASK_STACK_UI					((unsigned int *)0x20001400)
#define TASK_STACK_FREE				((unsigned int *)0x20001000)



#endif
