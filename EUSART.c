/*
 * File:   EUSART.c
 * Author: juanc
 *
 * Created on 4 de julio de 2020, 04:46 PM
 */


#include <xc.h>
#include "EUSART.h"
#define _XTAL_FREQ 20000000UL

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

void EUSART_TX(char dato) {
    TXREG = dato;
}

char EUSART_RX(void) {
    return RCREG;
}