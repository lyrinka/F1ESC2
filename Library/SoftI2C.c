#include <stm32f10x.h>
#include <Variables.h>
#include "GPIO.h"
#include "Task.h"
#include "SoftI2C.h"

// I2C MAC & LLC Layer
I2C_MAC_TypeDef I2C_MAC; 
I2C_LLC_TypeDef I2C_LLC; 

void I2C_PHY_Reset(void) {
	I2C_PHY_SCL_HIGH(); 
	I2C_PHY_SDA_HIGH(); 
}

void I2C_MAC_Reset(void) {
	I2C_MAC.mode = I2C_MAC_MODE_IDLE; 
	I2C_MAC.stage = 0; 
}

int I2C_MAC_Step(void) {
	int reset = 0; 
	switch(I2C_MAC.mode) {
		case I2C_MAC_MODE_IDLE: 
		default: {
			reset = 1; 
			break; 
		}
		case I2C_MAC_MODE_START: {
			switch(I2C_MAC.stage) {
				case 0: {
					I2C_PHY_SDA_LOW(); 
					break; 
				}
				case 1: {
					I2C_PHY_SCL_LOW(); 
				}
				default: {
					reset = 1; 
					break; 
				}
			}
			break; 
		}
		case I2C_MAC_MODE_RSTART: {
			switch(I2C_MAC.stage) {
				case 0: {
					I2C_PHY_SDA_HIGH(); 
					break; 
				}
				case 1: {
					I2C_PHY_SCL_HIGH(); 
					break; 
				}
				case 2: {
					I2C_PHY_SDA_LOW(); 
					break; 
				}
				case 3: {
					I2C_PHY_SCL_LOW(); 
				}
				default: {
					reset = 1; 
					break; 
				}
			}
			break; 	
		}
		case I2C_MAC_MODE_STOP: {
			switch(I2C_MAC.stage) {
				case 0: {
					I2C_PHY_SDA_LOW(); 
					break; 
				}
				case 1: {
					I2C_PHY_SCL_HIGH(); 
					break; 
				}
				case 2: {
					I2C_PHY_SDA_HIGH(); 
				}
				default: {
					reset = 1; 
					break; 
				}
			}
			break; 
		}
		case I2C_MAC_MODE_WRITE: {
			if(I2C_MAC.stage < 16) {
				if(I2C_MAC.stage & 0x1) {
					I2C_PHY_SCL_HIGH(); 
				}
				else {
					I2C_PHY_SCL_LOW(); 
					if(I2C_MAC.data & 0x80) 
						I2C_PHY_SDA_HIGH(); 
					else 
						I2C_PHY_SDA_LOW(); 
					I2C_MAC.data <<= 1; 
				}
			}
			else {
				switch(I2C_MAC.stage) {
					case 16: {
						I2C_PHY_SCL_LOW(); 
						I2C_PHY_SDA_HIGH(); 
						break; 
					}
					case 17: {
						I2C_PHY_SCL_HIGH(); 
						I2C_MAC.ack = I2C_PHY_SDA_READ(); 
						break; 
					}
					case 18: {
						I2C_PHY_SCL_LOW(); 
					}
					default: {
						reset = 1; 
						break; 
					}
				}
			}
			break; 
		}
		case I2C_MAC_MODE_READ: {
			if(I2C_MAC.stage == 0) 
				I2C_PHY_SDA_HIGH(); 
			if(I2C_MAC.stage < 16) {
				if(I2C_MAC.stage & 0x1) {
					I2C_PHY_SCL_HIGH(); 
					I2C_MAC.data <<= 1; 
					if(I2C_PHY_SDA_READ()) 
						I2C_MAC.data |= 0x1; 
				}
				else 
					I2C_PHY_SCL_LOW(); 
			}
			else {
				switch(I2C_MAC.stage) {
					case 16: {
						I2C_PHY_SCL_LOW(); 
						if(I2C_MAC.ack) 
							I2C_PHY_SDA_HIGH(); 
						else 
							I2C_PHY_SDA_LOW(); 
						break; 
					}
					case 17: {
						I2C_PHY_SCL_HIGH(); 
						break; 
					}
					case 18: {
						I2C_PHY_SCL_LOW(); 
					}
					default: {
						reset = 1; 
						break; 
					}
				}
			}
			break; 
		}
	}
	if(!reset) {
		I2C_MAC.stage++; 
	}
	else {
		I2C_MAC.mode = I2C_MAC_MODE_IDLE; 
		I2C_MAC.stage = 0; 
	}
	return reset; 
}

void I2C_LLC_Reset(void) {
	I2C_LLC.state = I2C_LLC_STATE_IDLE; 
	I2C_LLC.opcode_len = 0; 
	I2C_LLC.read_len = 0; 
	I2C_LLC.write_len = 0; 
}

int I2C_LLC_Step(void) {
	int reset = 0; 
	int exit = 1; 
	do {
		exit = 1; 
		switch(I2C_LLC.state) {
			default: 
			case I2C_LLC_STATE_IDLE: {
				reset = 1; 
				break; 
			}
			case I2C_LLC_STATE_START: {
				I2C_MAC.mode = I2C_MAC_MODE_START; 
				I2C_LLC.state = I2C_LLC_STATE_START_WAIT; 
			}
			case I2C_LLC_STATE_START_WAIT: {
				if(!I2C_MAC_Step()) break; 
				I2C_LLC.state = I2C_LLC_STATE_WADDRW; 
				break; 
			}
			case I2C_LLC_STATE_WADDRW: {
				if(!(I2C_LLC.mode & I2C_LLC_MODE_W)) {
					exit = 0; 
					I2C_LLC.state = I2C_LLC_STATE_WADDRR; 
					break; 
				}
				I2C_MAC.mode = I2C_MAC_MODE_WRITE; 
				I2C_MAC.data = I2C_LLC.devaddr & 0xFE; 
				I2C_LLC.state = I2C_LLC_STATE_WADDRW_WAIT; 
			}
			case I2C_LLC_STATE_WADDRW_WAIT: {
				if(!I2C_MAC_Step()) break; 
				I2C_LLC.state = I2C_LLC_STATE_WOP; 
				break; 
			}
			case I2C_LLC_STATE_WOP: {
				if(I2C_LLC.opcode_len == 0) {
					exit = 0; 
					I2C_LLC.state = I2C_LLC_STATE_WDATA; 
					break; 
				}
				I2C_LLC.index = I2C_LLC.opcode_len; 
				I2C_LLC.state = I2C_LLC_STATE_WOP_LOOP; 
			}
			case I2C_LLC_STATE_WOP_LOOP: {
				I2C_MAC.mode = I2C_MAC_MODE_WRITE; 
				I2C_MAC.data = (I2C_LLC.opcode >> (--I2C_LLC.index << 3)) & 0xFF; 
				I2C_LLC.state = I2C_LLC_STATE_WOP_WAIT; 
			}
			case I2C_LLC_STATE_WOP_WAIT: {
				if(!I2C_MAC_Step()) break; 
				if(I2C_LLC.index) {
					I2C_LLC.state = I2C_LLC_STATE_WOP_LOOP; 
					break; 
				}
				I2C_LLC.state = I2C_LLC_STATE_WDATA; 
				break; 
			}
			case I2C_LLC_STATE_WDATA: {
				if(I2C_LLC.write_len == 0) {
					exit = 0; 
					I2C_LLC.state = I2C_LLC_STATE_RSTART; 
					break; 
				}
				I2C_LLC.index = 0; 
				I2C_LLC.state = I2C_LLC_STATE_WDATA_LOOP; 
			}
			case I2C_LLC_STATE_WDATA_LOOP: {
				I2C_MAC.mode = I2C_MAC_MODE_WRITE; 
				I2C_MAC.data = I2C_LLC.write_buf[I2C_LLC.index]; 
				I2C_LLC.state = I2C_LLC_STATE_WDATA_WAIT; 
			}
			case I2C_LLC_STATE_WDATA_WAIT: {
				if(!I2C_MAC_Step()) break; 
				if(++I2C_LLC.index < I2C_LLC.write_len) {
					I2C_LLC.state = I2C_LLC_STATE_WDATA_LOOP; 
					break; 
				}
				I2C_LLC.state = I2C_LLC_STATE_RSTART; 
				break; 
			}
			case I2C_LLC_STATE_RSTART: {
				if(!(I2C_LLC.mode & I2C_LLC_MODE_R)) {
					exit = 0; 
					I2C_LLC.state = I2C_LLC_STATE_STOP; 
					break; 
				}
				I2C_MAC.mode = I2C_MAC_MODE_RSTART; 
				I2C_LLC.state = I2C_LLC_STATE_RSTART_WAIT; 
			}
			case I2C_LLC_STATE_RSTART_WAIT: {
				if(!I2C_MAC_Step()) break; 
				I2C_LLC.state = I2C_LLC_STATE_WADDRR; 
				break; 
			}
			case I2C_LLC_STATE_WADDRR: {
				I2C_MAC.mode = I2C_MAC_MODE_WRITE; 
				I2C_MAC.data = I2C_LLC.devaddr | 0x01; 
				I2C_LLC.state = I2C_LLC_STATE_WADDRR_WAIT; 
			}
			case I2C_LLC_STATE_WADDRR_WAIT: {
				if(!I2C_MAC_Step()) break; 
				I2C_LLC.state = I2C_LLC_STATE_RDATA; 
				break; 
			}
			case I2C_LLC_STATE_RDATA: {
				if(I2C_LLC.read_len == 0) {
					exit = 0; 
					I2C_LLC.state = I2C_LLC_STATE_STOP; 
					break; 
				}
				I2C_LLC.index = 0; 
				I2C_LLC.state = I2C_LLC_STATE_RDATA_LOOP; 
			}
			case I2C_LLC_STATE_RDATA_LOOP: {
				I2C_MAC.mode = I2C_MAC_MODE_READ; 
				I2C_MAC.ack = (I2C_LLC.index >= I2C_LLC.read_len - 1); 
				I2C_LLC.state = I2C_LLC_STATE_RDATA_WAIT; 
			}
			case I2C_LLC_STATE_RDATA_WAIT: {
				if(!I2C_MAC_Step()) break; 
				I2C_LLC.read_buf[I2C_LLC.index++] = I2C_MAC.data; 
				if(!I2C_MAC.ack) {
					I2C_LLC.state = I2C_LLC_STATE_RDATA_LOOP; 
					break; 
				}
				I2C_LLC.state = I2C_LLC_STATE_STOP; 
				break; 
			}
			case I2C_LLC_STATE_STOP: {
				if(I2C_LLC.mode & I2C_LLC_MODE_NSTOP) {
					reset = 1; 
					break; 
				}
				I2C_MAC.mode = I2C_MAC_MODE_STOP; 
				I2C_LLC.state = I2C_LLC_STATE_STOP_WAIT; 
			}
			case I2C_LLC_STATE_STOP_WAIT: {
				if(!I2C_MAC_Step()) break; 
				reset = 1; 
				break; 
			}
		}
	} while(!(exit || reset)); 
	if(reset) {
		I2C_LLC.state = I2C_LLC_STATE_IDLE; 
	}
	return reset; 
}

// I2C System Level Logic
int I2C_Busy = 0; 

void I2C_Init(void) {
	I2C_PHY_Reset(); 
	I2C_MAC_Reset(); 
	I2C_LLC_Reset(); 
	
	I2C_Busy = 0; 
	NVIC_SetPriority(SysTick_IRQn, PRIORITY_SYSTICK); 
	NVIC_ClearPendingIRQ(SysTick_IRQn); 
	NVIC_EnableIRQ(SysTick_IRQn); 
	
	SysTick->CTRL = 0x04; // Use 72MHz Kernel Clock
	SysTick->LOAD = 359; // 5us step delay, 100kHz
	SysTick->CTRL |= 0x01; 
}

int I2C_PromiseCallback(void) {
	return !I2C_Busy; 
}

int I2C_debugConflict = 0; 

void I2C_TransferWait(void) {
	Promise_TypeDef promise; 
	if(I2C_Busy) {
		Promise_Set(&promise, I2C_PromiseCallback); 
		await(&promise); 
	}
	if(I2C_Busy) 
		I2C_debugConflict = 1; 
}

void I2C_TransferTrigger(void) {
	I2C_LLC.state = I2C_LLC_STATE_START; 
	I2C_Busy = 1; 
	SysTick->CTRL |= 0x02; 
}

void I2C_Read(unsigned char addr, unsigned int len, unsigned char * buf) {
	I2C_TransferWait(); 
	
	I2C_LLC.devaddr = addr; 
	I2C_LLC.mode = I2C_LLC_MODE_R; 
	I2C_LLC.read_len = len; 
	I2C_LLC.read_buf = buf; 
	
	I2C_TransferTrigger(); 
	I2C_TransferWait(); 
}

void I2C_Write(unsigned char addr, unsigned int oplen, unsigned int op, unsigned int len, unsigned char * buf) {
	I2C_TransferWait(); 
	
	I2C_LLC.devaddr = addr; 
	I2C_LLC.mode = I2C_LLC_MODE_W; 
	if(oplen > 4) oplen = 4; 
	I2C_LLC.opcode_len = oplen; 
	I2C_LLC.opcode = op; 
	I2C_LLC.write_len = len; 
	I2C_LLC.write_buf = buf; 
	
	I2C_TransferTrigger(); 
	I2C_TransferWait(); 
}

void I2C_WriteRead(unsigned char addr, unsigned int oplen, unsigned int op, unsigned int wlen, unsigned char * wbuf, unsigned int rlen, unsigned char * rbuf) {
	I2C_TransferWait(); 
	
	I2C_LLC.devaddr = addr; 
	I2C_LLC.mode = I2C_LLC_MODE_WR; 
	if(oplen > 4) oplen = 4; 
	I2C_LLC.opcode_len = oplen; 
	I2C_LLC.opcode = op; 
	I2C_LLC.write_len = wlen; 
	I2C_LLC.write_buf = wbuf; 
	I2C_LLC.read_len = rlen; 
	I2C_LLC.read_buf = rbuf; 
	
	I2C_TransferTrigger(); 
	I2C_TransferWait(); 
}

// IRQ Handlers
void SysTick_Handler(void) {
	if(!I2C_LLC_Step()) return; 
	SysTick->CTRL &= ~0x02; 
	NVIC_ClearPendingIRQ(SysTick_IRQn); 
	I2C_Busy = 0; 
}
