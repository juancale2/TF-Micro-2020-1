/*
 * File:   main.c
 * Author: juanc
 *
 * Created on 7 de junio de 2020, 10:32 PM
 */


#include <xc.h>
#include <stdio.h>
#include <pic18f4550.h>
#include "Configuracion.h"
#include "sensores.h"
#include "LCD.h"
#include "EUSART.h"

//FUNCIONES
void config(void);

//VARIABLES
char buffer[5];
const unsigned char cCorazon[8] = {0x00, 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00};
unsigned int Oxigeno;
float Temperatura;
char Alarma = 3;
char Beat = 0;

void config(void) {
    //Configuracion de puertos
    TRISA = 0x07;
    TRISCbits.RC0 = 0;
    TRISCbits.RC1 = 1;
    TRISD = 0x00;
    
    //Configuracion del LCD
    LCD_CONFIG();
    CURSOR_ONOFF(OFF);
    GENERACARACTER(cCorazon, 1);
    
    //Configuracion del TMR0
    T0CON = 0x03;
    INTCONbits.TMR0IE = 1;
    //Configuracion del TMR1
    T1CON = 0x81;
    TMR1 = 0xD8F0;
    PIE1bits.TMR1IE = 1;
    //Configuracion global de interrupciones
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    //Configuracion de comunicacion EUSART
    EUSART_Init(9600);
    
    return;
}

void main(void) {
    config();
    while(1) {     
        //Envia frecuencia cardiaca al LCD
        POS_CURSOR(1,0);
        ESCRIBE_MENSAJE("BPM:",4);
        sprintf(buffer,"%3u",BPM);
        ESCRIBE_MENSAJE(buffer,3);
        
        //Envia saturacion de oxigeno al LCD
        POS_CURSOR(2,0);
        ESCRIBE_MENSAJE("SpO2:",5);
        sprintf(buffer,"%3u",Oxigeno);
        ESCRIBE_MENSAJE(buffer,3);
        ESCRIBE_MENSAJE("%",1);
        
        //Envia temperatura corporal al LCD
        POS_CURSOR(1,9);
        ESCRIBE_MENSAJE("T:",2);
        sprintf(buffer,"%.1f",Temperatura);
        ESCRIBE_MENSAJE(buffer,4);
        ESCRIBE_MENSAJE("C",1);
        
        //Envia caracter de corazon al reconocer un pulso
        POS_CURSOR(2,15);
        if(Beat)
            ENVIA_CHAR(1); 
        else
            ENVIA_CHAR(' ');
        
        //Envio de datos mediante protocolo EUSART solo si detecta un cambio
        //en el nivel de alerta segun los parametros
        
        //Alerta Grave - 3:
        //BPM menor a 60 (Bradicardia)
        //Oxigeno menor a 60% (Hipoxemia grave)
        if(BPM <= 60 || Oxigeno <= 60) {
            if(Alarma != 3) {
                Alarma = 3;
                EUSART_TX(3);
            }
        }
        //Alerta Media - 2:
        //BPM mayor a 100 (Taquicardia)
        //Oxigeno menor a 80% (Hipoxemia media-leve)
        else if(BPM >= 100 || Oxigeno <= 80) {
            if(Alarma != 2) {
                Alarma = 2;
                EUSART_TX(2);
            }
        }
        //Alerta Baja - 1:
        //Temperatura mayor 38 (Fiebre)
        else if(Temperatura > 38) {
            if(Alarma != 1) {
                Alarma = 1;
                EUSART_TX(1);
            }
        }
        //Sin alerta - 0
        //Parametros normales
        else if(Alarma != 0){
            Alarma = 0;
            EUSART_TX(0);
        }
        
        __delay_ms(50); 
    }
    return;
}

void __interrupt() interrupcion(void) {
    
    //Interrupcion por TMR1 configurada para desbordar cada 2ms
    if(PIR1bits.TMR1IF) {
        PIR1bits.TMR1IF = 0;
        samplePulse();     //Funcion que muestrea la señal cardiaca
        if(QS == true) {   //QS se activa cuando se detectó un latido
            QS = false;    //Se baja la bandera QS
            Beat = 1;      //Variable del programa principal para saber si ocurrió un latido
            TMR0 = 0x0BDB; //Se configura TMR0 para que detenga la señal de latido luego de 200ms
            T0CONbits.TMR0ON = 1;
            if(Alarma >= 2 || PORTCbits.RC1)  //Si la alarma es mayor a "Media" o RC1 = 1
                LATCbits.LATC0 = 1;   //Se activa la señal sonora que indica un latido
        }
        
        if(BPM < 30)             //Si frecuencia cardiaca es menor a 30 BPM
            LATCbits.LATC0 = 1;  //Se activa la alerta sonora de señal cardiaca plana o paro cardiaco
        
        Oxigeno = getOxi();      //Se muestrea la saturacion de oxigeno
        Temperatura = getTemp(); //Se muestrea la temperatura corporal
        
        TMR1 = 0xD8F0;           //TMR1 se reinicia para 2ms
    }
    
    //Interrupcion por TMR0 configurada para reiniciar la señal de pulso despues de 200ms
    if(INTCONbits.TMR0IF){
        INTCONbits.TMR0IF = 0;
        Beat = 0;
        LATCbits.LATC0 = 0;
        T0CONbits.TMR0ON = 0;
    }
    
    return;
}
