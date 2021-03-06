
#ifndef __I2C_H
#define __I2C_H

// Define i2c pins
#define SDA	PORTCbits.RC4				// Data pin for i2c
#define SCK	PORTCbits.RC3				// Clock pin for i2c
#define SDA_DIR	TRISCbits.TRISC4			// Data pin direction
#define SCK_DIR	TRISCbits.TRISC3			// Clock pin direction

// Define i2c speed
#define I2C_SPEED	100				// kbps

//Function Declarations
void InitI2C(void);
void I2C_Start(void);
void I2C_ReStart(void);
void I2C_Stop(void);
void I2C_Send_ACK(void);
void I2C_Send_NACK(void);
char  I2C_Write_Byte(unsigned char);
unsigned char I2C_Read_Byte(void);

#endif
