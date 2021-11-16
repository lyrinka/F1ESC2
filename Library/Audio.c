#include <Buzzer.h>
#include "Audio.h"

const unsigned short Audio_Freq_Table[89] = {
	  275,   291,   309, 
    327,   346,   367,   389,   412,   437,   462,   490,   519,   550,   583,   617, 
    654,   693,   734,   778,   824,   873,   925,   980,  1038,  1100,  1165,  1235, 
	 1308,  1386,  1468,  1556,  1648,  1746,  1850,  1960,  2077,  2200,  2331,  2469, 
	 2616,  2772,  2937,  3111,  3296,  3492,  3700,  3920,  4153,  4400,  4662,  4939, 
	 5233,  5544,  5873,  6223,  6593,  6985,  7400,  7840,  8306,  8800,  9323,  9878, 
	10465, 11087, 11747, 12445, 13185, 13969, 14800, 15680, 16612, 17600, 18647, 19755, 
	20930, 22175, 23493, 24890, 26370, 27938, 29600, 31360, 33224, 35200, 37293, 39511, 
	41860, 
	    0, 
}; 

#define NOTE_A_TRANS -3
#define NOTE_B_TRANS -1
#define NOTE_C_TRANS 0
#define NOTE_D_TRANS 2
#define NOTE_E_TRANS 4
#define NOTE_F_TRANS 5
#define NOTE_G_TRANS 7

#define NOTE_REST 88
#define NOTE_C (0  + 51)
#define NOTE_D (2  + 51)
#define NOTE_E (4  + 51)
#define NOTE_F (5  + 51)
#define NOTE_G (7  + 51)
#define NOTE_A (9  + 51)
#define NOTE_B (11 + 51)

#define NOTE_SHARP 1
#define NOTE_FLAT -1

#define NOTE_OCTAVE_0 -60
#define NOTE_OCTAVE_1 -48
#define NOTE_OCTAVE_2 -36
#define NOTE_OCTAVE_3 -24
#define NOTE_OCTAVE_4 -12
#define NOTE_OCTAVE_5  0
#define NOTE_OCTAVE_6  12
#define NOTE_OCTAVE_7  24
#define NOTE_OCTAVE_8  36

#define NOTE_OCTAVE_UP    12
#define NOTE_OCTAVE_DOWN -12

#define AUDIO_TRANS NOTE_D_TRANS

void Audio_Buffer_Shadow(unsigned int length, const unsigned char * note, unsigned short * buf) {
	for(int i = 0; i < length; i++) {
		int temp = note[i]; 
		if(temp != 88) temp += AUDIO_TRANS; 
		buf[i] = Audio_Freq_Table[temp]; 
	}
}

void Audio_Play(unsigned int length, const unsigned char * note, const unsigned short * time, unsigned short * buf) {
	if(length <= 0) return; 
	Audio_Buffer_Shadow(length, note, buf); 
	Buzzer_Play(length, buf, time); 
}

// Declared Buffers
unsigned short Audio_Motor_Buffer[16]; 
unsigned short Audio_UI_Buffer[16]; 

// Declared songs

/*
const unsigned int Audio_What_What_Length = 0; 
const unsigned char Audio_What_What_Note[] = {
	
}; 
const unsigned short Audio_What_What_Time[] = {
	
}; 
*/

// Motor Task Songs
const unsigned int Audio_Motor_Beep_Length = 4; 
const unsigned char Audio_Motor_Beep_Note[] = {
	NOTE_G + NOTE_OCTAVE_DOWN, NOTE_REST, NOTE_G + NOTE_OCTAVE_DOWN, NOTE_REST, 
}; 
const unsigned short Audio_Motor_Beep_Time[] = {
	50, 150, 50, 100, 
}; 


// UI Task Songs
const unsigned int Audio_UI_Startup_Length = 5; 
const unsigned char Audio_UI_Startup_Note[] = {
	NOTE_C, NOTE_D, NOTE_E, NOTE_G, NOTE_REST, 
}; 
const unsigned short Audio_UI_Startup_Time[] = {
	100, 100, 100, 150, 500, 
}; 
/*
const unsigned int Audio_UI__Length = 0; 
const unsigned char Audio_UI__Note[] = {
	
}; 
const unsigned short Audio_UI__Time[] = {
	
}; 
*/
const unsigned int Audio_UI_TakenOver_Enter_Length = 5; 
const unsigned char Audio_UI_TakenOver_Enter_Note[] = {
	NOTE_C, NOTE_D, NOTE_E, NOTE_G, NOTE_REST, 
}; 
const unsigned short Audio_UI_TakenOver_Enter_Time[] = {
	100, 100, 100, 150, 200, 
}; 

const unsigned int Audio_UI_TakenOver_Exit_Length = 5; 
const unsigned char Audio_UI_TakenOver_Exit_Note[] = {
	NOTE_G, NOTE_E, NOTE_D, NOTE_C, NOTE_REST, 
}; 
const unsigned short Audio_UI_TakenOver_Exit_Time[] = {
	100, 100, 100, 150, 200, 
}; 

const unsigned int Audio_UI_ForceRun_Enter_Length = 5; 
const unsigned char Audio_UI_ForceRun_Enter_Note[] = {
	NOTE_C + NOTE_OCTAVE_UP, NOTE_G, NOTE_C + NOTE_OCTAVE_UP, NOTE_G, NOTE_REST
}; 
const unsigned short Audio_UI_ForceRun_Enter_Time[] = {
	200, 200, 200, 200, 200
}; 

const unsigned int Audio_UI_ForceRun_Exit_Length = 4; 
const unsigned char Audio_UI_ForceRun_Exit_Note[] = {
	NOTE_E, NOTE_D, NOTE_C, NOTE_REST, 
}; 
const unsigned short Audio_UI_ForceRun_Exit_Time[] = {
	100, 100, 100, 100, 
}; 

const unsigned int Audio_UI_Fault_Enter_Length = 6; 
const unsigned char Audio_UI_Fault_Enter_Note[] = {
	NOTE_C, NOTE_C + NOTE_OCTAVE_UP, NOTE_REST, NOTE_C, NOTE_C + NOTE_OCTAVE_UP, NOTE_REST, 
}; 
const unsigned short Audio_UI_Fault_Enter_Time[] = {
	50, 50, 100, 50, 50, 100
}; 

const unsigned int Audio_UI_Fault_Exit_Length = 5; 
const unsigned char Audio_UI_Fault_Exit_Note[] = {
	NOTE_G + NOTE_OCTAVE_DOWN, NOTE_A + NOTE_OCTAVE_DOWN, NOTE_G + NOTE_OCTAVE_DOWN, NOTE_C, NOTE_REST, 
}; 
const unsigned short Audio_UI_Fault_Exit_Time[] = {
	100, 100, 100, 700, 100, 
}; 

const unsigned int Audio_UI_Fault_LoseSync_Enter_Length = 8; 
const unsigned char Audio_UI_Fault_LoseSync_Enter_Note[] = {
	NOTE_C, NOTE_REST, NOTE_C, NOTE_REST, NOTE_C, NOTE_REST, NOTE_C, NOTE_REST, 
}; 
const unsigned short Audio_UI_Fault_LoseSync_Enter_Time[] = {
	50, 50, 50, 50, 50, 50, 50, 100, 
}; 

const unsigned int Audio_UI_Fault_Hot_Enter_Length = 4; 
const unsigned char Audio_UI_Fault_Hot_Enter_Note[] = {
	NOTE_E + NOTE_OCTAVE_UP, NOTE_REST, NOTE_C + NOTE_OCTAVE_UP, NOTE_REST, 
}; 
const unsigned short Audio_UI_Fault_Hot_Enter_Time[] = {
	50, 50, 50, 150, 
}; 

const unsigned int Audio_UI_Fault_Power_Enter_Length = 5; 
const unsigned char Audio_UI_Fault_Power_Enter_Note[] = {
	NOTE_C, NOTE_G + NOTE_OCTAVE_DOWN, NOTE_E + NOTE_OCTAVE_DOWN, NOTE_C + NOTE_OCTAVE_DOWN, NOTE_REST, 
}; 
const unsigned short Audio_UI_Fault_Power_Enter_Time[] = {
	100, 100, 100, 400, 100, 
}; 

const unsigned int Audio_UI_Fault_Stuck_Enter_Length = 5; 
const unsigned char Audio_UI_Fault_Stuck_Enter_Note[] = {
	NOTE_C, NOTE_D, NOTE_C, NOTE_D, NOTE_REST, 
}; 
const unsigned short Audio_UI_Fault_Stuck_Enter_Time[] = {
	100, 100, 100, 100, 200, 
}; 

const unsigned int Audio_UI_Fault_Tilt_Enter_Length = 5; 
const unsigned char Audio_UI_Fault_Tilt_Enter_Note[] = {
	NOTE_C, NOTE_B + NOTE_OCTAVE_DOWN, NOTE_A + NOTE_OCTAVE_DOWN, NOTE_G + NOTE_OCTAVE_DOWN, NOTE_REST, 
}; 
const unsigned short Audio_UI_Fault_Tilt_Enter_Time[] = {
	100, 100, 100, 100, 200, 
}; 
