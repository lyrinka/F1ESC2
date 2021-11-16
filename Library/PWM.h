#ifndef __PWM_H__
#define __PWM_H__

extern volatile unsigned int PWM_CurrentCycle; 
extern volatile unsigned int PWM_UpdateFlag; 

extern void PWM_Init(void); 
extern int PWM_BlockingWaitCycle(void); 

#define PWM_Enable() TIM1->BDTR |= 0x8000
#define PWM_Disable() TIM1->BDTR &= ~0x8000
#define PWM_Set(ch, val) (&TIM1->CCR1)[(ch) << 1] = val

#endif
