
/*
 * File:   main.c
 * Author: hdask
 *
 * Created on 12 de octubre de 2021, 11:36 AM
 */


//CONFIG
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = ON        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
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
    RW = 0;
    RS = 0;
    E = 1;
    LCD = dato;
    E = 0;
    __delay_ms(2);
        
}

void SEND_CHAR(char dato)
{
    RW = 0;
    RS = 1;
    E = 1;
    LCD = dato;
    E=0;
    __delay_ms(1);
}

void LCD_Init() 
{
    __delay_ms(20); //Esperar LCD
    SEND_CMD(0x38); // Function Set
    SEND_CMD(0x0C); //Display ON, Cursor off
    SEND_CMD(0x06); //Entry mode set
    SEND_CMD(0x01); //Borra LCD
}

void EscribeCadenaLCD (const char *s)
{
    while(*s)
        SEND_CHAR(*s++); //encia al lcd mientras no null
    
}

void UART_Init()
{
    //Iniciamos el modulo UART 9600bps 
    TXSTA = 0x26; //00100110
    RCSTA = 0x90; //10010000
    SPBRG = 25;
    PIR1bits.TXIF = 0;
    PIR1bits.RCIF = 0;
    // MCU_Init();
}

void MCU_Init()
{
    TRISC= 0xB8; //RS RW E DE SALIDA
    TRISB = 0x00; //TODO EL PUERTO PARA LCD DE SALIDA 
    TRISD = 0xF0;
}

void SEND_Tx(char dato)
{
    while(TXSTAbits.TRMT == 0){};
    TXREG = dato;
}

void MSG_Term(const char *s)
{
while(*s) SEND_Tx(*s++); //envia al UART mientras no null
    SEND_Tx(0x0D);
    SEND_Tx(0x0A);
}

void RecibeHHMM()
{
    char Car, Datos[4];
    SEND_CMD(0x01); //Borra LCD
    
    MSG_Term("Escriba la Hora:");
    
    while(PIR1bits.RCIF == 0);
    if(PIR1bits.RCIF == 1)
        {
            Car = RCREG; //Captura dato del UART
            PIR1bits.RCIF = 0; //borra bandera UART recibida
            Datos[3] = Car;
            SEND_Tx(Car);
        }
    while(PIR1bits.RCIF == 0);
    if(PIR1bits.RCIF == 1)
        {
            Car = RCREG; //Captura dato del UART
            PIR1bits.RCIF = 0; //borra bandera UART recibida
            Datos[2] = Car;
            SEND_Tx(Car);
            SEND_Tx(0x0D);
            SEND_Tx(0x0A);
        } 
    
    MSG_Term("Escriba los minutos:");
    while(PIR1bits.RCIF == 0);
    if (PIR1bits.RCIF == 1)
        {
            Car = RCREG; //captura dato del UART
            PIR1bits.RCIF = 0; //borra bandera del UART recibido
            Datos[1] = Car;
            SEND_Tx(Car);
        }
    
    while(PIR1bits.RCIF == 0);
    if(PIR1bits.RCIF == 1)
        {
            Car = RCREG; 
            PIR1bits.RCIF = 0;
            Datos[0] = Car;
            SEND_Tx(Car);
            SEND_Tx(0x0D);
            SEND_Tx(0x0A);
        }
    
    //Escribimos esta hora 
    
    Write_Byte_To_DS1307_RTC(2,(Datos[3]<<4)+ (Datos[2]& 0x0F)); //Horas
    Write_Byte_To_DS1307_RTC(1,(Datos[1]<<4)+ (Datos[0]& 0x0F)); //Minutos
    Write_Byte_To_DS1307_RTC(0,0); //Segundos
    
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
        if (PORTDbits.RD6 == 0) goto Antirrebote;
        Tecla++;
        if (PORTDbits.RD7 == 0) goto Antirrebote;
        Tecla++;
        VPTOD = (char)(VPTOD << 1) | 1;
        __delay_ms(10);

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
            return '/';
        case 5:
            return '4';
        case 6:
            return '5';
        case 7:
            return '6';
        case 8:
            return 'X';
        case 9: 
            return '1';
        case 10:
            return '2';
        case 11:
            return '3';
        case 12:
            return '-';
        case 13:
            return 0x0D;
        case 14:
            return '0';
        case 15:
            return '=';
        case 16:
            return '+';
        default:
            break;

    }
    return 0;
}


void LEA_FECHA()
{
    char fecha[6];
    //char i = 0;
    SEND_CMD(0x01); //Borra LCD
    
    EscribeCadenaLCD("Teclee d?a (DD):");
    char car=0;
    while (car == 0){
        car = TECLADO();
        __delay_ms(10);
    }
    SEND_CHAR(car);
    fecha[0] = car;
    
    car = 0;
    while (car == 0){
        car = TECLADO(); 
        __delay_ms(10);
    }
    SEND_CHAR(car);
    fecha[1] = car;
    
    __delay_ms(200);
    
    //mes
    SEND_CMD(0x01); //Borra LCD
    
    EscribeCadenaLCD("Teclee mes (MM):");
    
    
    car = 0;
    while (car == 0){
        car = TECLADO();
        __delay_ms(10);
        
    }
    SEND_CHAR(car);
    fecha[3] = car;
    __delay_ms(200);
    
    //a?o
    SEND_CMD(0x01); //borra LCD
    
    EscribeCadenaLCD("Teclee a?o (AA):");
    car = 0;
    while (car == 0){
        car = TECLADO(); 
        __delay_ms(10);
       
    }
    
    SEND_CHAR(car);
    fecha[4] = car;
    
    car = 0;
    while (car == 0){
        car = TECLADO(); 
        __delay_ms(10);
    }
    
    SEND_CHAR(car);
    fecha[5] = car;
    __delay_ms(200);
    
    SEND_CMD(1);
    
    char DIA = (fecha[0]<<4) + (fecha[1]&0x0F);
    Write_Byte_To_DS1307_RTC(4,DIA);
    
    char MES = (fecha[2]<<4) + (fecha[3]&0x0F);
    Write_Byte_To_DS1307_RTC(5,MES);
    
    char ANNO = (fecha[4]<<4) + (fecha[5]&0x0F);
    Write_Byte_To_DS1307_RTC(6,ANNO);

}

//*****************************

void ESCRIBA_SEE(char addr, char dato)
{
#define ADD_24LC 0xA0
    
    I2C_Start();
    
    //send i2c address of 24lc32 
    while(I2C_Write_Byte(ADD_24LC + 0) == 1)
    { I2C_Start(); }
    
    I2C_Write_Byte(0); //parte alta en cero
    I2C_Write_Byte(addr);
    I2C_Write_Byte(dato);
    I2C_Stop();
}

// *************** 

char LEA_SEE(char Address)

{
    unsigned char Byte = 0;
    
    I2C_Start();
    
    while (I2C_Write_Byte(ADD_24LC + 0) == 1)
    { I2C_Start(); }
    
    I2C_Write_Byte(0); 
    I2C_Write_Byte(Address);
    I2C_ReStart();
    
    I2C_Write_Byte(ADD_24LC+ 1);
    
    Byte = I2C_Read_Byte();
    
    I2C_Send_NACK();
    I2C_Stop();
    
    return Byte;
}

// *************** 

void main(void)
{
    char Car;
    MCU_Init();
    LCD_Init();
    UART_Init();
    InitI2C();
    
    EscribeCadenaLCD("MPEI 2020-i + I2C");
    
    SEND_CMD(0x80 + 64); //Pasa a la segunda l?nea
    EscribeCadenaLCD(" +++ RTC DS1307 +++");
    
    SEND_CMD(0x80+20); //Pasa a la tercera linea //1001 0100
    EscribeCadenaLCD("Lectura y escritura");
    
    SEND_CMD(0x80+84); //pasa a la cuarta linea 1101 0100
    EscribeCadenaLCD("DEL MODULO RTC:...");
    
    __delay_ms(2000);
    
    SEND_CMD(1); //BORRA LCD
    
    while(1)
    {
        SEND_CMD(0x80+0); //pasa a la linea 1,col 1;
        
        Car = Read_Byte_From_DS1307_RTC(2);
        SEND_CHAR(((Car>>4) & 0x0F)+0x30);
        SEND_CHAR((Car & 0x0F) + 0x30);
        
        SEND_CHAR(':');
        
        Car = Read_Byte_From_DS1307_RTC(1);
        SEND_CHAR(((Car>>4) & 0x0F)+0x30);
        SEND_CHAR((Car & 0x0F) + 0x30);
        
        SEND_CHAR(':');
        
        Car = Read_Byte_From_DS1307_RTC(0);
        SEND_CHAR(((Car>>4) & 0x0F)+0x30);
        SEND_CHAR((Car & 0x0F) + 0x30);
        
        SEND_CMD(0xC0);
        
        Car = Read_Byte_From_DS1307_RTC(4); //DIA
        SEND_CHAR(((Car>>4) & 0x03)+0x30);
        SEND_CHAR((Car & 0x0F) + 0x30);
        
        SEND_CHAR('/');
        
        Car = Read_Byte_From_DS1307_RTC(5); //MES
        switch(Car)
        {
            case 1:
                EscribeCadenaLCD("ENE");
                break;
            case 2:
                EscribeCadenaLCD("FEB");
                break;
            case 3:
                EscribeCadenaLCD("MAR");
                break;
            case 4:
                EscribeCadenaLCD("ABR");
                break;
            case 5:
                EscribeCadenaLCD("MAY");
                break;
            case 6:
                EscribeCadenaLCD("JUN");
                break;
            case 7:
                EscribeCadenaLCD("JUL");
                break;
            case 8:
                EscribeCadenaLCD("AGO");
                break;
            case 9:
                EscribeCadenaLCD("SEP");
                break;
            case 0x10:
                EscribeCadenaLCD("OCT");
                break;
            case 0x11:
                EscribeCadenaLCD("NOV");
                break;
            case 0x12:
                EscribeCadenaLCD("DIC");
                break;
            default:
                SEND_CHAR(((Car>>4) & 0x0F) + 0x30);
                SEND_CHAR((Car & 0x0F) + 0x30);
                break;
                
        }
        
        SEND_CHAR('/');
        
        Car = Read_Byte_From_DS1307_RTC(6); //A?o
        SEND_CHAR(((Car >> 4) & 0x0F) + 0x30);
        SEND_CHAR((Car & 0x0F) + 0x30);
        
        if(PIR1bits.RCIF == 1)
            {
                Car = RCREG; // captura dato del UART
                PIR1bits.RCIF = 0; //Borra bandera UART recibida
                if(Car == 'H') RecibeHHMM();
            }
        
        __delay_ms(100);
        
        char a = TECLADO();
        if(a == 0x0D)
        {
            LEA_FECHA();
            //ESCRIBA_SEE(0x07,'D'); //tiene que ser comillas sencillas
            __delay_ms(10);
            //a = LEA_SEE(0x07);
            SEND_CMD(0x80 + 20); //pasar tercera linea
            SEND_CHAR(a);
        }
    }
}
