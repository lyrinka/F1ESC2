#ifndef __BUZZER_H__
#define __BUZZER_H__

typedef struct {
	int length; 
	int index; 
	const unsigned short * freq; 
	const unsigned short * time; 
} Buzzer_Sequence_Typedef; 

extern void Buzzer_Init(void); 

extern void Buzzer_Play(int, const unsigned short *, const unsigned short *); 
extern void Buzzer_PlayAsync(int, const unsigned short *, const unsigned short *); 

#endif
