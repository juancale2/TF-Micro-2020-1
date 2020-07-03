//#include <p18F4550.h>
#include <xc.h>
#include "ADC.h"


void ADC_CONFIG(char canal)
{
    ADCON0=canal<<2;
	ADCON1=0x0C;
	ADCON2 = 0x95; 	//4TAD, FOSC/16, justificación a la derecha
	ADCON0bits.ADON = 1;
}
unsigned int ADC_CONVERTIR(void)
{
	int num;
	ADCON0bits.GO = 1;
	while(ADCON0bits.GO);
	num = 0x03 & ADRESH;
	num<<= 8;
	num +=ADRESL;
	return(num);
}
