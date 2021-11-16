#ifndef __ADS7830_H__
#define __ADS7830_H__

#define ADS7830_CH0_1	0x0
#define ADS7830_CH2_3	0x1
#define ADS7830_CH4_5	0x2
#define ADS7830_CH6_7	0x3
#define ADS7830_CH1_0	0x4
#define ADS7830_CH3_2	0x5
#define ADS7830_CH5_4	0x6
#define ADS7830_CH7_6	0x7
#define ADS7830_CH0		0x8
#define ADS7830_CH2		0x9
#define ADS7830_CH4		0xA
#define ADS7830_CH6		0xB
#define ADS7830_CH1		0xC
#define ADS7830_CH3		0xD
#define ADS7830_CH5		0xE
#define ADS7830_CH7		0xF

extern unsigned char ADS7830_Read(unsigned char, unsigned char); 

#endif
