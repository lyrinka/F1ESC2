#ifndef __DS2482_H__
#define __DS2482_H__

extern void DS2482_Init(unsigned char); 
extern int DS2482_1Wire_Reset(unsigned char); 
extern void DS2482_1Wire_Write(unsigned char, unsigned char); 
extern unsigned char DS2482_1Wire_Read(unsigned char); 
extern void DS2482_1Wire_WriteBit(unsigned char, int); 
extern int DS2482_1Wire_ReadBit(unsigned char); 
extern int DS2482_1Wire_Triplet(unsigned char, int); 

#endif
