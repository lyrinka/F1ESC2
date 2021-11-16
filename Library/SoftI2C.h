
#ifndef __SoftI2C_H__
#define __SoftI2C_H__

#define I2C_MAC_MODE_IDLE 0
#define I2C_MAC_MODE_START 1
#define I2C_MAC_MODE_RSTART 2
#define I2C_MAC_MODE_STOP 3
#define I2C_MAC_MODE_WRITE 4
#define I2C_MAC_MODE_READ 5

#define I2C_LLC_MODE_W 1
#define I2C_LLC_MODE_R 2
#define I2C_LLC_MODE_WR 3
#define I2C_LLC_MODE_NSTOP 4

#define I2C_LLC_STATE_IDLE 0
#define I2C_LLC_STATE_START 1
#define I2C_LLC_STATE_START_WAIT 2
#define I2C_LLC_STATE_WADDRW 3
#define I2C_LLC_STATE_WADDRW_WAIT 4
#define I2C_LLC_STATE_WOP 5
#define I2C_LLC_STATE_WOP_LOOP 6
#define I2C_LLC_STATE_WOP_WAIT 7
#define I2C_LLC_STATE_WDATA 8
#define I2C_LLC_STATE_WDATA_LOOP 9
#define I2C_LLC_STATE_WDATA_WAIT 10
#define I2C_LLC_STATE_RSTART 11
#define I2C_LLC_STATE_RSTART_WAIT 12
#define I2C_LLC_STATE_WADDRR 13
#define I2C_LLC_STATE_WADDRR_WAIT 14
#define I2C_LLC_STATE_RDATA 15
#define I2C_LLC_STATE_RDATA_LOOP 16
#define I2C_LLC_STATE_RDATA_WAIT 17
#define I2C_LLC_STATE_STOP 18
#define I2C_LLC_STATE_STOP_WAIT 19

typedef struct {
	unsigned char mode; 
	unsigned char stage; 
	unsigned char ack; 
	unsigned char data; 
} I2C_MAC_TypeDef; 

typedef struct {
	unsigned char devaddr; 
	unsigned char state; 
	unsigned char mode; 
	unsigned char opcode_len; 
	unsigned int opcode; 
	unsigned char * write_buf; 
	unsigned int write_len; 
	unsigned char * read_buf; 
	unsigned int read_len; 
	unsigned int index; 
} I2C_LLC_TypeDef; 

extern I2C_MAC_TypeDef I2C_MAC; 
extern I2C_LLC_TypeDef I2C_LLC; 
extern int I2C_Pending; 

extern void I2C_PHY_Reset(void); 

extern void I2C_MAC_Reset(void); 
extern int I2C_MAC_Step(void); 

extern void I2C_LLC_Reset(void); 
extern int I2C_LLC_Step(void); 

extern void I2C_Init(void); 

extern void I2C_Read(unsigned char, unsigned int, unsigned char *); 
extern void I2C_Write(unsigned char, unsigned int, unsigned int, unsigned int, unsigned char *); 
extern void I2C_WriteRead(unsigned char, unsigned int, unsigned int, unsigned int, unsigned char *, unsigned int, unsigned char *); 

#endif
