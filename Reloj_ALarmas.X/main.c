/*
 * File:   main.c
 * Author: Danilo A Garcia H
 *
 * Created on 14 de julio de 2020, 12:46 AM
 */


// CONFIG
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = ON         // Data EEPROM Memory Code Protection bit (Data EEPROM code-protected)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


#define _XTAL_FREQ 4000000

#include <xc.h>
#include "I2C.c"
#include "DS1307.c"


#define RS PORTCbits.RC0
#define E  PORTCbits.RC2
#define RW PORTCbits.RC1
#define LCD PORTB

void SEND_CMD(char dato)
{
    RW=0;
    RS=0;
    E=1;
    LCD=dato;
    E=0;
    __delay_ms(2);
}

void SEND_CHAR(char dato)
{
    RW=0;
    RS=1;
    E=1;
    LCD=dato;
    E=0;
    __delay_ms(1);   
}

void LCD_Init()   //ver p谩gina 24 manual HD44780
{
    __delay_ms(20); //esperar LCD
    SEND_CMD(0x38); //Function Set
    SEND_CMD(0x0C); //Display ON, Cursor off
    SEND_CMD(0x06); //Entry mode Set
    SEND_CMD(0x01); //Borra LCD
}

void EscribeCadenaLCD(const char *s)
{
	while(*s)
		SEND_CHAR(*s++);   // env铆a al LCD mientras no NULL 
}


void UART_Init()
{
    //iniciamos el modulo UART 9600bps 8N1 
    TXSTA=0x26;     //00100110
    RCSTA=0x90;     //10010000
    SPBRG=25;
    PIR1bits.TXIF=0;
    PIR1bits.RCIF=0;
}

void MCU_Init()
{
    TRISC=0xF8; //RS RW E DE SALIDA
    TRISB=0X00; //TODO EL PUERTO PARA LCD DE SALIDA
}

void SEND_Tx(char dato)
{
    while(TXSTAbits.TRMT==0){};
    TXREG=dato;        
}

void MSG_Term(const char *s)
{
while(*s) SEND_Tx(*s++);   // env韆 al UART mientras no NULL 
    SEND_Tx(0x0D);
    SEND_Tx(0x0A);
}

void RecibeHHMM()
{     char Car, Datos[4];
SEND_CMD(0x01);     //Borra LCD

    MSG_Term("Escriba la Hora:");
    
    while(PIR1bits.RCIF==0);
    if(PIR1bits.RCIF==1) 
            {
                Car=RCREG;          //captura dato del UART
                PIR1bits.RCIF=0;    //borra bandera UART recibi贸
                Datos[3]=Car;
                SEND_Tx(Car);
            }
    
    while(PIR1bits.RCIF==0);
    if(PIR1bits.RCIF==1) 
            {
                Car=RCREG;          //captura dato del UART
                PIR1bits.RCIF=0;    //borra bandera UART recibi贸
                Datos[2]=Car;
                SEND_Tx(Car);
                SEND_Tx(0x0D);
                SEND_Tx(0x0A);
            }
    
    MSG_Term("Escriba los minutos:");
    while(PIR1bits.RCIF==0);
    if(PIR1bits.RCIF==1) 
            {
                Car=RCREG;          //captura dato del UART
                PIR1bits.RCIF=0;    //borra bandera UART recibi贸
                Datos[1]=Car;
                SEND_Tx(Car);
            }
    
    while(PIR1bits.RCIF==0);
    if(PIR1bits.RCIF==1) 
            {
                Car=RCREG;          //captura dato del UART
                PIR1bits.RCIF=0;    //borra bandera UART recibi贸
                Datos[0]=Car;
                SEND_Tx(Car);
                SEND_Tx(0x0D);
                SEND_Tx(0x0A);
            }
    
    //escribimos esta hora
        
    Write_Byte_To_DS1307_RTC(2,(Datos[3]<<4)+(Datos[2]&0x0F)); //horas
    Write_Byte_To_DS1307_RTC(1,(Datos[1]<<4)+(Datos[0]&0x0F) ); //minutos
    Write_Byte_To_DS1307_RTC(0,0); //segundos
}

void main(void) {
    char Car;
        
    MCU_Init();
    LCD_Init();
    UART_Init();
    InitI2C();	
    
    EscribeCadenaLCD(" MPEI 2020-i + I2C  ");
    
    SEND_CMD(0x80+64); //pasa a la segunda linea
    EscribeCadenaLCD(" +++ RTC DS1307 +++ ");
    
    SEND_CMD(0x80+20); //pasa a la tercera linea  //1001 0100
    EscribeCadenaLCD("LECTURA Y ESCRITURA ");
    
    SEND_CMD(0x80+84); //pasa a la cuarta linea  1101 0100
    EscribeCadenaLCD("DEL MODULO RTC :... ");
    
    SEND_CMD(1);        //borra la LCD
    
    while(1)
    {
    SEND_CMD(0x80+0); //pasa a 1a linea 1, col 1
    
    Car=Read_Byte_From_DS1307_RTC(2);
    SEND_CHAR(((Car>>4)&0x0F)+0x30);
    SEND_CHAR((Car&0x0F)+0x30);
    
    SEND_CHAR(':');
    
    Car=Read_Byte_From_DS1307_RTC(1);
    SEND_CHAR(((Car>>4)&0x0F)+0x30);
    SEND_CHAR((Car&0x0F)+0x30);
    
    SEND_CHAR(':');
    
    Car=Read_Byte_From_DS1307_RTC(0);
    SEND_CHAR(((Car>>4)&0x0F)+0x30);
    SEND_CHAR((Car&0x0F)+0x30);
    
    if(PIR1bits.RCIF==1) 
            {
                Car=RCREG;          //captura dato del UART
                PIR1bits.RCIF=0;    //borra bandera UART recibi贸
                if(Car=='H') RecibeHHMM();
            }
    
     __delay_ms(1000);
      
    }
}
