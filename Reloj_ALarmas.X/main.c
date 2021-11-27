/*
 * File:   main.c
 * Author: Andreina y santiago
 *
 * Created on 16 november 2021, 12:46 AM
 */
// HOLA MUNDO, ESTOY HACIENDO UN CAMBIO A LA NUBE DE TRABAJO
// HOLA hola

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
#include "I2C.h"
#include "DS1307.h"

#define RS PORTCbits.RC0
#define E  PORTCbits.RC2
#define RW PORTCbits.RC1
#define SCL PORTCbits.RC3
#define SDA PORTCbits.RC4
#define Buzz PORTCbits.RC5
#define LCD PORTB
#define ADD_24LC 0xA0



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

void LCD_Init()   //ver pÃ¡gina 24 manual HD44780
{
    __delay_ms(20); //esperar LCD
    SEND_CMD(0x38); //Function Set
    SEND_CMD(0x0C); //Display ON, Cursor off
    SEND_CMD(0x06); //Entry mode Set
    SEND_CMD(0x01); //Borra LCD
}

void SEND_MSJ(char POS, char *Msj)
{
    char carac;
    SEND_CMD(POS);
    while(*Msj!=0x00){
        carac=(char)*Msj;
        SEND_CHAR(carac);
        Msj++;
    }
    return;
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
    TRISC=0x19; //RS RW E DE SALIDA
    TRISB=0x00; //TODO EL PUERTO PARA LCD DE SALIDA
    TRISD=0x0F;
}
void Init_TMR2(void) 
{
    T2CONbits.TMR2ON = 1; //PRENDE EL TIMER2
    PIE1bits.TMR2IE = 1; //HABILITA INTERRUPCION DEL TIMER 2
    INTCONbits.PEIE = 1; //HABILITA INTERRUPCION DE PERIFERICOS
    INTCONbits.GIE = 1; //HABILITA INTERRUPCIONES DE FORMA GLOBAL
    T2CONbits.T2CKPS = 0x3;
    T2CONbits.TOUTPS = 0x0F;
}

void __interrupt() VIS_DIN(void) {
    if (PIE1bits.TMR2IE && PIR1bits.TMR2IF) //EVALUA BIT DE HAB Y BANDERA DEL TMR2  
    {   
        PR2=0xC3;       //Inicializa el conteo máximo.


        PIR1bits.TMR2IF = 0; //BORRA LA BANDERA DEL TIMER 2
    }
}

void SEND_Tx(char dato)
{
    while(TXSTAbits.TRMT==0){};
    TXREG=dato;        
}

void MSG_Term(const char *s)
{
while(*s) SEND_Tx(*s++);   // envía al UART mientras no NULL 
    SEND_Tx(0x0D);
    SEND_Tx(0x0A);
}

char TECLADO(){
    char Tecla = 1;
    char VPTOD = 0x0E; //0000 1110
    
    do
    {
        PORTD = VPTOD;
        if (PORTDbits.RD4 == 0) goto Antirrebote;
        Tecla++;
        if (PORTDbits.RD5 == 0) goto Antirrebote;
        Tecla++;
        if (PORTDbits.RD7 == 0) goto Antirrebote;
        Tecla++;
        VPTOD = (char) ((VPTOD << 1) | 1);
  
    } while (Tecla < 17);
    PORTD = 0xFF;
    return 0;

    Antirrebote:
    while(PORTDbits.RD4 == 0) {};
    while(PORTDbits.RD5 == 0) {};
    while(PORTDbits.RD6 == 0) {};
    while(PORTDbits.RD7 == 0) {};
    __delay_ms(100);
    PORTD = 0xFF;
    switch(Tecla){
        case 1:
            return '7';
        case 2:
            return '8';
        case 3:
            return '9';
        case 4:
            return 0x0A;
        case 5:
            return '4';
        case 6:
            return '5';
        case 7:
            return '6';
        case 8:
            return 0x0B;
        case 9: 
            return '1';
        case 10:
            return '2';
        case 11:
            return '3';
        case 12:
            return 0x0C;
        case 13:
            return 0x0D;
        case 14:
            return '0';
        case 15:
            return 0x0E;
        case 16:
            return 0x0F;
            
    }
    return 0;   
}
void deco_LCD_dia(char sel){
    switch (sel){
        case 0:
            SEND_MSJ(0xC0,"Lunes");
            return;
            break;
        case 1:
            SEND_MSJ(0xC0,"Martes");
            return;
            break;
        case 2:
            SEND_MSJ(0xC0,"Miercoles");
            return;
            break;
        case 3:
            SEND_MSJ(0xC0,"Jueves");
            return;
            break;
        case 4:
            SEND_MSJ(0xC0,"Viernes");
            return;
            break;
        case 5:
            SEND_MSJ(0xC0,"Sabado");
            return;
            break;
        case 6:
            SEND_MSJ(0xC0,"Domingo");
            return;
            break;
        default:
            return;
            break;
    }
}
void EscribeDHHMM()
{     char  Datos[5];
    SEND_CMD(0x01);     //Borra LCD

    

    SEND_MSJ(0x80,"Marque el dia");
    SEND_MSJ(0xC0,"Lun:1 Mar:2 Mirc:3");
    SEND_MSJ(0x80+20,"Juv:4 Vir:5 Sab:6");
    SEND_MSJ(0xC0+20,"Dom:7");
    SEND_CMD(1);
    Datos[4]=TECLADO;
    SEND_MSJ(0x80+20,"Escriba :");
    SEND_MSJ(0xC0+20,"Hora y minutos:");
    __delay_ms(1500);
    Datos[3]=TECLADO();
    Datos[2]=TECLADO();
    Datos[1]=TECLADO();
    Datos[0]=TECLADO();
    deco_LCD_dia(Datos[4]);
    SEND_CMD(0x80+11);
    SEND_CHAR(Datos[3]);
    SEND_CHAR(Datos[2]);
    SEND_CHAR(':');
    SEND_CHAR(Datos[1]);
    SEND_CHAR(Datos[0]); 
    __delay_ms(3000);
    
        
    //escribimos este momento
    Write_Byte_To_DS1307_RTC(3,Datos[4]); //Día
    Write_Byte_To_DS1307_RTC(2,(Datos[3]<<4)+(Datos[2]&0x0F)); //horas
    Write_Byte_To_DS1307_RTC(1,(Datos[1]<<4)+(Datos[0]&0x0F) ); //minutos
    Write_Byte_To_DS1307_RTC(0,0); //segundos
}


void ESCRIBA_SEE(char addr, char dato)
{
    
    I2C_Start();
    
    //send i2c address of 24lc32 
    while(I2C_Write_Byte(ADD_24LC + 0) == 1)
    { I2C_Start(); }
    
    I2C_Write_Byte(0); //parte alta en cero
    I2C_Write_Byte(addr);
    I2C_Write_Byte(dato);
    I2C_Stop();
}
char LEA_SEE(char addr)

{
    unsigned char Byte = 0;
    
    I2C_Start();
    
    while (I2C_Write_Byte(ADD_24LC + 0) == 1)
    { I2C_Start(); }
    
    I2C_Write_Byte(0); 
    I2C_Write_Byte(addr);
    I2C_ReStart();
    
    I2C_Write_Byte(ADD_24LC+ 1);
    
    Byte = I2C_Read_Byte();
    
    I2C_Send_NACK();
    I2C_Stop();
    
    return (char)Byte;
}


main(void) {
    
}
   