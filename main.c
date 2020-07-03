/*
 * File:   main.c
 * Author: juanc
 *
 * Created on 7 de junio de 2020, 10:32 PM
 */


#include <xc.h>
#include <stdio.h>
#include "Configuracion.h"
#include "sensores.h"
#include "LCD.h"

char buffer[5];
const unsigned char cCorazon[8] = {0x00, 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00};
unsigned int Oxigeno;
float Temperatura;
char Alarma = 4;
void config(void);
void EUSART_Init(long BAUD);

void config(void) {
    TRISA = 0x07;
    TRISD = 0x00;
    TRISCbits.RC0 = 0;
    LCD_CONFIG();
    CURSOR_ONOFF(OFF);
    GENERACARACTER(cCorazon, 1);
    T0CON = 0x03;
    T1CON = 0x81;
    TMR1 = 0xD8F0;
    PIE1bits.TMR1IE = 1;
    INTCONbits.TMR0IE = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    EUSART_Init(9600);
    return;
}
void main(void) {
    config();
    while(1) {     
        POS_CURSOR(1,0);
        ESCRIBE_MENSAJE("BPM:",4);
        sprintf(buffer,"%3u",BPM);
        ESCRIBE_MENSAJE(buffer,3);
        
        POS_CURSOR(2,0);
        ESCRIBE_MENSAJE("SpO2:",5);
        sprintf(buffer,"%3u",Oxigeno);
        ESCRIBE_MENSAJE(buffer,3);
        ESCRIBE_MENSAJE("%",1);
        
        POS_CURSOR(1,9);
        ESCRIBE_MENSAJE("T:",2);
        sprintf(buffer,"%.1f",Temperatura);
        ESCRIBE_MENSAJE(buffer,4);
        ESCRIBE_MENSAJE("C",1);
        
        POS_CURSOR(2,15);
        if(PORTCbits.RC0)
            ENVIA_CHAR(1); 
        else
            ENVIA_CHAR(' ');
        
        if(BPM <= 60 || Oxigeno <= 60) {
            if(Alarma != 3) {
                Alarma = 3;
                TXREG = 3;
            }
        }
        else if(BPM >= 100 || Oxigeno <= 80) {
            if(Alarma != 2) {
                Alarma = 2;
                TXREG = 2;
            }
        }
        else if(Temperatura > 38) {
            if(Alarma != 1) {
                Alarma = 1;
                TXREG = 1;
            }
        }
        else if(Alarma != 0){
            Alarma = 0;
            TXREG = 0;
        }
        
        __delay_ms(20); 
    }
    return;
}

void EUSART_Init(long BAUD) {
    TRISCbits.RC6 = 0;
    TRISCbits.RC7 = 1;
    SPBRG = (unsigned char)(((_XTAL_FREQ/BAUD)/64)-1);
    
    TXSTAbits.BRGH = 0;
    TXSTAbits.SYNC = 0;
    RCSTAbits.SPEN = 1;
    
    TXSTAbits.TX9 = 0;
    TXSTAbits.TXEN = 1;
    
    RCSTAbits.RC9 = 0;
    RCSTAbits.CREN = 1;
}

void __interrupt() interrupcion(void) {
    if(PIR1bits.TMR1IF) {
        PIR1bits.TMR1IF = 0;
        samplePulse();
        if(QS == true) {
            QS = false;
            LATCbits.LATC0 = 1;
            TMR0 = 0x0BDB; //150ms
            T0CONbits.TMR0ON = 1;
        }
        if(BPM < 30)
            LATCbits.LATC0 = 1;
        
        Oxigeno = getOxi();
        Temperatura = getTemp();
        TMR1 = 0xD8F0;
    }
    
    if(INTCONbits.TMR0IF){
        INTCONbits.TMR0IF = 0;
        LATCbits.LATC0 = 0;
        T0CONbits.TMR0ON = 0;
    }
    
    return;
}
