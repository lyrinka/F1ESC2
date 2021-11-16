#ifndef __GPIO_H__
#define __GPIO_H__
/* 
	Pin configurations: 
	PA0  Pu          - External Pins
	PA1  Pu          - External Pins
	PA2  AFPPXM      - External Pins (IBus Telemetry)
	PA3  Pu          - External Pins
	PA4  Pu          - External Pins
	PA5  Pu          - External Pins
	PA6  Pu          - External Pins
	PA7  Pu          - External Pins
	PA8  AFPP2M T1C1 - Motor A PWM, Active High
	PA9  AFPP2M T1C2 - Motor B PWM, Active High
	PA10 AFPP2M T1C3 - Motor C PWM, Active High
	PA11 AFPP2M T1C4 - Motor D PWM, Active High
	PA12 IOPP2M GPIO - Speed Sensor Enable, Active Low
	PA13             - Debug SWDIO
	PA14             - Debug SWCLK
	PA15 AFPP2M T2C1 - Buzzer, Active High
	PB0  Float  T3C3 - Motor C Speed, Active Low
	PB1  Float  T3C4 - Motor D Speed, Active Low
	PB2  Pu     EXTI - Over Current Fault Signal, Active Low
	PB3  Pd     EXTI - Tilt Fault Signal, Active High
	PB4  Float  T3C1 - Motor A Speed, Active Low
	PB5  Float  T3C2 - Motor B Speed, Active Low
	PB6  AFPPXM U1Tx - Serial TxD (IBus Control)
	PB7  Pu     U1Rx - Serial RxD
	PB8  Float  bCAN - CAN Bus RxD
	PB9  AFPPXM bCAN - CAN Bus TxD
	PB10 IOOD2M GPIO - SCL
	PB11 IOOD2M GPIO - SDA
	PB12 Pd     T1BK - Short Circuit Break Signal, Active Low
	PB13 IOPP2M GPIO - Temperature Sensor Enable, Active Low
	PB14 IOPP2M GPIO - Master Switch Enable, Active High
	PB15 IOPP2M GPIO - Backup Switch Enable, Active High
	PC13 IOPP2M GPIO - Link LED (blue) Signal, Active High
	PC14 IOPP2M GPIO - Act LED (red) Signal, Active High
	PC15 Float  EXTI - User Pushbutton Signal, Active Low
	PD0              - HSE
	PD1              - HSE
*/
extern void GPIO_Init(void); 
// Functional Switches
#define GPIO_TEMP_EN() GPIOB->BRR = 0x2000
#define GPIO_TEMP_DIS() GPIOB->BSRR = 0x2000
#define GPIO_SPD_EN() GPIOA->BRR = 0x1000
#define GPIO_SPD_DIS() GPIOA->BSRR = 0x1000
#define GPIO_SW1_EN() GPIOB->BSRR = 0x8000
#define GPIO_SW1_DIS() GPIOB->BRR = 0x8000
#define GPIO_SW2_EN() GPIOB->BSRR = 0x4000
#define GPIO_SW2_DIS() GPIOB->BRR = 0x4000
// User Inferface
#define GPIO_LED_LINK_ON() GPIOC->BSRR = 0x2000
#define GPIO_LED_LINK_OFF() GPIOC->BRR = 0x2000
#define GPIO_LED_ACT_ON() GPIOC->BSRR = 0x4000
#define GPIO_LED_ACT_OFF() GPIOC->BRR = 0x4000
#define GPIO_KEY_READ() (GPIOC->IDR & 0x8000)
// I2C Interface
#define I2C_PHY_SCL_HIGH() GPIOB->BSRR = 0x0400
#define I2C_PHY_SCL_LOW() GPIOB->BRR = 0x0400
#define I2C_PHY_SDA_HIGH() GPIOB->BSRR = 0x0800
#define I2C_PHY_SDA_LOW() GPIOB->BRR = 0x0800
#define I2C_PHY_SDA_READ() (GPIOB->IDR & 0x0800)
#endif
