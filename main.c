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

float buffer[10];
char cCorazon[8] = {0x00, 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00};

void config(void) {
    TRISD = 0x00;
    TRISC = 0x00;
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
        sprintf(buffer,"%3u",getOxi());
        ESCRIBE_MENSAJE(buffer,3);
        ESCRIBE_MENSAJE("%",1);
        
        POS_CURSOR(1,9);
        ESCRIBE_MENSAJE("T:",2);
        sprintf(buffer,"%.1f",getTemp());
        ESCRIBE_MENSAJE(buffer,4);
        ESCRIBE_MENSAJE("C",1);
        
        POS_CURSOR(2,15);
        if(PORTCbits.RC0)
            ENVIA_CHAR(1); 
        else
            ENVIA_CHAR(' ');
        
        __delay_ms(100); 
    }
    return;
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
        TMR1 = 0xD8F0;
    }
    
    if(INTCONbits.TMR0IF){
        INTCONbits.TMR0IF = 0;
        LATCbits.LATC0 = 0;
        T0CONbits.TMR0ON = 0;
    }
    
    return;
}
