
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
#define ADD_24LC 0xA0
#define Buzz PORTCbits.RC5

void SEND_CMD(char dato) {
    RW = 0;
    RS = 0;
    E = 1;
    LCD = dato;
    E = 0;
    __delay_ms(2);

}

void SEND_CHAR(char dato) {
    RW = 0;
    RS = 1;
    E = 1;
    LCD = dato;
    E = 0;
    __delay_ms(1);
}

void LCD_Init() {
    __delay_ms(20); //Esperar LCD
    SEND_CMD(0x38); // Function Set
    SEND_CMD(0x0C); //Display ON, Cursor off
    SEND_CMD(0x06); //Entry mode set
    SEND_CMD(0x01); //Borra LCD
}

void SEND_MSJ(char POS, char *Msj) {
    char carac;
    SEND_CMD(POS);
    while (*Msj != 0x00) {
        carac = (char) *Msj;
        SEND_CHAR(carac);
        Msj++;
    }

}

void UART_Init() {
    //Iniciamos el modulo UART 9600bps 
    TXSTA = 0x26; //00100110
    RCSTA = 0x90; //10010000
    SPBRG = 25;
    PIR1bits.TXIF = 0;
    PIR1bits.RCIF = 0;
    // MCU_Init();
}

void MCU_Init() {
    TRISC = 0xB8; //RS RW E DE SALIDA
    TRISB = 0x00; //TODO EL PUERTO PARA LCD DE SALIDA 
    TRISD = 0xF0;
}

void SEND_Tx(char dato) {
    while (TXSTAbits.TRMT == 0) {
    };
    TXREG = dato;
}

void MSG_Term(const char *s) {
    while (*s) SEND_Tx(*s++); //envia al UART mientras no null
    SEND_Tx(0x0D);
    SEND_Tx(0x0A);
}
char TECLADO() {
    char Tecla = 1;
    char VPTOD = 0x0E; //0000 1110

    do {
        PORTD = VPTOD;
        if (PORTDbits.RD4 == 0) goto Antirrebote;
        Tecla++;
        if (PORTDbits.RD5 == 0) goto Antirrebote;
        Tecla++;
        if (PORTDbits.RD6 == 0) goto Antirrebote;
        Tecla++;
        if (PORTDbits.RD7 == 0) goto Antirrebote;
        Tecla++;
        VPTOD = (char) (VPTOD << 1) | 1;
        __delay_ms(10);

    } while (Tecla < 17);
    PORTD = 0xFF;
    return 0;

Antirrebote:
    while (PORTDbits.RD4 == 0) {
    };
    while (PORTDbits.RD5 == 0) {
    };
    while (PORTDbits.RD6 == 0) {
    };
    while (PORTDbits.RD7 == 0) {
    };
    __delay_ms(100);
    PORTD = 0xFF;
    switch (Tecla) {
        case 1:
            return '1';
        case 2:
            return '2';
        case 3:
            return '3';
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
            return '7';
        case 10:
            return '8';
        case 11:
            return '9';
        case 12:
            return 0x0C;
        case 13:
            return 'n';
        case 14:
            return '0';
        case 15:
            return 'f';
        case 16:
            return 0x0D;
        default:
            break;

    }
    return 0;
}


void RecibeHHMM() {
    char Car, Datos[4];
    SEND_CMD(0x01); //Borra LCD

    SEND_MSJ(0x80, "Escriba la Hora:");
    SEND_MSJ(0x80, "HH/MM");

    Car = TECLADO();
    Datos[3] = Car;
    Car = TECLADO();
    Datos[2] = Car;
    Car = TECLADO();
    Datos[1] = Car;
    Car = TECLADO();
    Datos[0] = Car;
    SEND_CMD(0x80 + 20);
    SEND_CHAR(Datos[3]);
    SEND_CHAR(Datos[2]);
    SEND_CHAR(':');
    SEND_CHAR(Datos[1]);
    SEND_CHAR(Datos[0]);
    //Escribimos esta hora 

    Write_Byte_To_DS1307_RTC(2, (Datos[3] << 4)+ (Datos[2]& 0x0F)); //Horas
    Write_Byte_To_DS1307_RTC(1, (Datos[1] << 4)+ (Datos[0]& 0x0F)); //Minutos
    Write_Byte_To_DS1307_RTC(0, 0); //Segundos

}

char Deco_num(char dato) {
    switch (dato) {
        case 0:
            return '0';
        case 1:
            return '1';
        case 2:
            return '2';
        case 3:
            return '3';
        case 4:
            return '4';
        case 5:
            return '5';
        case 6:
            return '6';
        case 7:
            return '7';
        case 8:
            return '8';
        case 9:
            return '9';
        default:
            return ' ';
    }
}
void deco_mes(char addr,char Car) {
    switch (Car) {
        case 1:
            SEND_MSJ(addr, "ENE");
            break;
        case 2:
            SEND_MSJ(addr, "FEB");
            break;
        case 3:
            SEND_MSJ(addr, "MAR");
            break;
        case 4:
            SEND_MSJ(addr, "ABR");
            break;
        case 5:
            SEND_MSJ(addr, "MAY");
            break;
        case 6:
            SEND_MSJ(addr, "JUN");
            break;
        case 7:
            SEND_MSJ(addr, "JUL");
            break;
        case 8:
            SEND_MSJ(addr, "AGO");
            break;
        case 9:
            SEND_MSJ(addr, "SEP");
            break;
        case 0x10:
            SEND_MSJ(addr, "OCT");
            break;
        case 0x11:
            SEND_MSJ(addr, "NOV");
            break;
        case 0x12:
            SEND_MSJ(addr, "DIC");
            break;
        default:
            SEND_CHAR(((Car >> 4) & 0x0F) + 0x30);
            SEND_CHAR((Car & 0x0F) + 0x30);
            break;

    }
}

//*****************************************************************************
void ESCRIBA_SEE(char addr, char dato) {

    I2C_Start();

    //send i2c address of 24lc32 
    while (I2C_Write_Byte(ADD_24LC + 0) == 1) {
        I2C_Start();
    }

    I2C_Write_Byte(0); //parte alta en cero
    I2C_Write_Byte(addr);
    I2C_Write_Byte(dato);
    I2C_Stop();
}
//******************************************************************************


void RecibeALARMAS(char addr) {
    char Car, Datos[10], aux1, aux2,ctr;
    SEND_CMD(0x01); //Borra LCD

    SEND_MSJ(0x80, "Escriba alarma ");
    aux1 = addr / 10;
    aux2 = aux1 * 10 - addr;
    SEND_CHAR(Deco_num(aux1));
    SEND_CHAR(Deco_num(aux2));
    SEND_MSJ(0xC0, "AA/MM/DD HH:MM");
    SEND_MSJ(0x80+20, "*=on #=off ");
    SEND_CHAR('a');
    __delay_ms(2000);
    SEND_CMD(1);
    //Año
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[9] = Car;
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[8] = Car;
    //Mes
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[7] = Car;
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[6] = Car;
    //Dia
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[5] = Car;
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[4] = Car;
    //Hora
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[3] = Car;
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[2] = Car;
    //Minutos
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[1] = Car;
    Car = TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    Datos[0] = Car;
    //ctr
    Car=TECLADO();
    while (Car == 0){
        Car = TECLADO();
        __delay_ms(10);
    }
    
    if (Car == 'n'){
        ctr = 1;
    }
    else{
        ctr = 0;
    }
    
//    switch(Car){
//        case 'n':
//            ctr=1;
//            break;
//        case 'f':
//            ctr=0;
//            break;
//        default:
//            ctr=0;
//            break;
//    }
    SEND_CMD(0x80);
    //Hora
    SEND_CHAR(Datos[3]);
    SEND_CHAR(Datos[2]);
    SEND_CHAR(':');
    //Minutos
    SEND_CHAR(Datos[1]);
    SEND_CHAR(Datos[0]);
    
    if (ctr==1){
        SEND_MSJ(0x80+15,"On");
    }else {
        SEND_MSJ(0x80+15,"Off");
    }
    //Dia
    SEND_CMD(0xC0);
    SEND_CHAR(Datos[5]);
    SEND_CHAR(Datos[4]);
    SEND_CHAR('/');
    //Mes
    deco_mes(0xC0 + 2, ((Datos[7] << 4)+ (Datos[6]& 0x0F)));
    SEND_CHAR('/');
    //Año
    SEND_CHAR(Datos[9]);
    SEND_CHAR(Datos[8]);
    //Escribimos esta hora 
    ESCRIBA_SEE(addr++, (Datos[3] << 4)+ (Datos[2]& 0x0F)); //Hora
    __delay_ms(10);
    ESCRIBA_SEE(addr++, (Datos[1] << 4)+ (Datos[0]& 0x0F)); //Minutos    
    __delay_ms(10);
    ESCRIBA_SEE(addr++, (Datos[5] << 4)+ (Datos[4]& 0x0F)); //Dia
    __delay_ms(10);
    ESCRIBA_SEE(addr++, (Datos[7] << 4)+ (Datos[6]& 0x0F)); //Mes    
    __delay_ms(10);
    ESCRIBA_SEE(addr++, (Datos[9] << 4)+ (Datos[8]& 0x0F)); //Año
    __delay_ms(10);
    ESCRIBA_SEE(addr++, ctr); //Control   
    __delay_ms(10);
    return;
}


void LEA_FECHA() {
    char fecha[6];
    //char i = 0;
    SEND_CMD(0x01); //Borra LCD

    SEND_MSJ(0x80, "Teclee día (DD):");
    char car = 0;
    while (car == 0) {
        car = TECLADO();
        __delay_ms(10);
    }
    SEND_CHAR(car);
    fecha[0] = car;

    car = 0;
    while (car == 0) {
        car = TECLADO();
        __delay_ms(10);
    }
    SEND_CHAR(car);
    fecha[1] = car;

    __delay_ms(200);

    //mes
    SEND_CMD(0x01); //Borra LCD

    SEND_MSJ(0x80, "Teclee mes (MM):");


    car = 0;
    while (car == 0) {
        car = TECLADO();
        __delay_ms(10);

    }
    SEND_CHAR(car);
    fecha[3] = car;
    __delay_ms(200);

    //año
    SEND_CMD(0x01); //borra LCD

    SEND_MSJ(0x80,"Teclee año (AA):");
    car = 0;
    while (car == 0) {
        car = TECLADO();
        __delay_ms(10);

    }

    SEND_CHAR(car);
    fecha[4] = car;

    car = 0;
    while (car == 0) {
        car = TECLADO();
        __delay_ms(10);
    }

    SEND_CHAR(car);
    fecha[5] = car;
    __delay_ms(200);

    SEND_CMD(1);

    char DIA = (fecha[0] << 4) + (fecha[1]&0x0F);
    Write_Byte_To_DS1307_RTC(4, DIA);

    char MES = (fecha[2] << 4) + (fecha[3]&0x0F);
    Write_Byte_To_DS1307_RTC(5, MES);

    char ANNO = (fecha[4] << 4) + (fecha[5]&0x0F);
    Write_Byte_To_DS1307_RTC(6, ANNO);

}



char LEA_SEE(char Address)
 {
    unsigned char Byte = 0;

    I2C_Start();

    while (I2C_Write_Byte(ADD_24LC + 0) == 1) {
        I2C_Start();
    }

    I2C_Write_Byte(0);
    I2C_Write_Byte(Address);
    I2C_ReStart();

    I2C_Write_Byte(ADD_24LC + 1);

    Byte = I2C_Read_Byte();

    I2C_Send_NACK();
    I2C_Stop();

    return Byte;
}

// *************** 


void deco_dia(char sel) {
    switch (sel) {
        case 0:
            SEND_MSJ(0xC0, "Lunes");
            return;
            break;
        case 1:
            SEND_MSJ(0xC0, "Martes");
            return;
            break;
        case 2:
            SEND_MSJ(0xC0, "Miercoles");
            return;
            break;
        case 3:
            SEND_MSJ(0xC0, "Jueves");
            return;
            break;
        case 4:
            SEND_MSJ(0xC0, "Viernes");
            return;
            break;
        case 5:
            SEND_MSJ(0xC0, "Sabado");
            return;
            break;
        case 6:
            SEND_MSJ(0xC0, "Domingo");
            return;
            break;
        default:
            return;
            break;
    }
}

void anti_r() {
    char Car;
    while (Car != 'r') {
        if (PIR1bits.RCIF == 1) {

            Car = RCREG; // captura dato del UART
            PIR1bits.RCIF = 0; //Borra bandera UART recibida
        }
    }
    return;
}
void Buzz_on(){
    Buzz=0;
}
void Buzz_off(){
    Buzz=1;
}
void melodia(){
    
    Buzz_on();
    __delay_ms(300);
    Buzz_off();
    __delay_ms(700);
}

void main(void) {
    char Car;
    MCU_Init();
    LCD_Init();
    UART_Init();
    InitI2C();

    SEND_MSJ(0x80,"MPEI LAB 6");

    SEND_MSJ(0xC0 , "    RTC DS1307   ");
    //Pasa a la tercera linea //1001 0100
    SEND_MSJ(0x80 + 20, "Lectura y escritura");
    //pasa a la cuarta linea 1101 0100
    SEND_MSJ(0xC0 + 20, "ALARMAS EEPROM");

    __delay_ms(2000);

    SEND_CMD(1); //BORRA LCD
    MSG_Term("Hasta acá llegué");
    while (1) {
        if (PIR1bits.RCIF == 1) {
            Car = RCREG; // captura dato del UART
            PIR1bits.RCIF = 0; //Borra bandera UART recibida
            MSG_Term("Hasta acá llegué");
        
        switch (Car){
            case '1': 
                anti_r();
                RecibeALARMAS(0);
                MSG_Term("1");
               
            case '2':
                anti_r();
                RecibeALARMAS(8);
                               
            case '3':
                anti_r();
                RecibeALARMAS(16);               
            case '4':
                anti_r();
                RecibeALARMAS(24);               
            case '5':
                anti_r();
                RecibeALARMAS(32);               
            case '6':
                anti_r();
                RecibeALARMAS(40);               
            case '7':
                anti_r();
                RecibeALARMAS(48);               
            case '8':
                anti_r();
                RecibeALARMAS(56);               
            case '9':
                anti_r();
                RecibeALARMAS(64);               
            case '0':
                anti_r();
                RecibeALARMAS(72);
            }
        }
        
        
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

        deco_mes(0xC0+4,(Car));
        SEND_CHAR('/');
        
        Car = Read_Byte_From_DS1307_RTC(6); //Año
        SEND_CHAR(((Car >> 4) & 0x0F) + 0x30);
        SEND_CHAR((Car & 0x0F) + 0x30);

        
        __delay_ms(1000);
        

    }
}