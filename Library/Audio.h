#ifndef __AUDIO_H__
#define __AUDIO_H__

#define Audio_PlayNamed(module, name) Audio_Play(Audio_##module##_##name##_Length, Audio_##module##_##name##_Note, Audio_##module##_##name##_Time, Audio_##module##_Buffer) 

extern void Audio_Buffer_Shadow(unsigned int, const unsigned char *, unsigned short *); 
extern void Audio_Play(unsigned int, const unsigned char *, const unsigned short *, unsigned short *); 

// Declared Buffers
extern unsigned short Audio_Motor_Buffer[]; 
extern unsigned short Audio_UI_Buffer[]; 

// Declared songs
extern const unsigned int Audio_Motor_Beep_Length; 
extern const unsigned char Audio_Motor_Beep_Note[]; 
extern const unsigned short Audio_Motor_Beep_Time[]; 

extern const unsigned int Audio_UI_Startup_Length; 
extern const unsigned char Audio_UI_Startup_Note[]; 
extern const unsigned short Audio_UI_Startup_Time[]; 

extern const unsigned int Audio_UI_TakenOver_Enter_Length; 
extern const unsigned char Audio_UI_TakenOver_Enter_Note[]; 
extern const unsigned short Audio_UI_TakenOver_Enter_Time[]; 

extern const unsigned int Audio_UI_TakenOver_Exit_Length; 
extern const unsigned char Audio_UI_TakenOver_Exit_Note[]; 
extern const unsigned short Audio_UI_TakenOver_Exit_Time[]; 

extern const unsigned int Audio_UI_ForceRun_Enter_Length; 
extern const unsigned char Audio_UI_ForceRun_Enter_Note[]; 
extern const unsigned short Audio_UI_ForceRun_Enter_Time[]; 

extern const unsigned int Audio_UI_ForceRun_Exit_Length; 
extern const unsigned char Audio_UI_ForceRun_Exit_Note[]; 
extern const unsigned short Audio_UI_ForceRun_Exit_Time[]; 

extern const unsigned int Audio_UI_Fault_Enter_Length; 
extern const unsigned char Audio_UI_Fault_Enter_Note[]; 
extern const unsigned short Audio_UI_Fault_Enter_Time[]; 

extern const unsigned int Audio_UI_Fault_Exit_Length; 
extern const unsigned char Audio_UI_Fault_Exit_Note[]; 
extern const unsigned short Audio_UI_Fault_Exit_Time[]; 

extern const unsigned int Audio_UI_Fault_LoseSync_Enter_Length; 
extern const unsigned char Audio_UI_Fault_LoseSync_Enter_Note[]; 
extern const unsigned short Audio_UI_Fault_LoseSync_Enter_Time[]; 

extern const unsigned int Audio_UI_Fault_Hot_Enter_Length; 
extern const unsigned char Audio_UI_Fault_Hot_Enter_Note[]; 
extern const unsigned short Audio_UI_Fault_Hot_Enter_Time[]; 

extern const unsigned int Audio_UI_Fault_Power_Enter_Length; 
extern const unsigned char Audio_UI_Fault_Power_Enter_Note[]; 
extern const unsigned short Audio_UI_Fault_Power_Enter_Time[]; 

extern const unsigned int Audio_UI_Fault_Stuck_Enter_Length; 
extern const unsigned char Audio_UI_Fault_Stuck_Enter_Note[]; 
extern const unsigned short Audio_UI_Fault_Stuck_Enter_Time[]; 

extern const unsigned int Audio_UI_Fault_Tilt_Enter_Length; 
extern const unsigned char Audio_UI_Fault_Tilt_Enter_Note[]; 
extern const unsigned short Audio_UI_Fault_Tilt_Enter_Time[]; 


#endif
