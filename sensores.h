#include <xc.h>
#include "ADC.h"

#define PULSIMETRO 0
#define OXIMETRO 1
#define TERMOMETRO 2

typedef unsigned char bool;
#define true    1
#define false   0


void samplePulse(void);
unsigned int getSignal(char sensor);
unsigned int getOxi(void);
float getTemp(void);

volatile int rate[10]; // array to hold last ten IBI values
volatile unsigned long sampleCounter = 0; // used to determine pulse timing
volatile unsigned long lastBeatTime = 0; // used to find IBI
volatile int P = 512; // used to find peak in pulse wave, seeded
volatile int T = 512; // used to find trough in pulse wave, seeded
volatile int thresh = 525; // used to find instant moment of heart beat, seeded
volatile int amp = 0; // used to hold amplitude of pulse waveform, seeded
volatile bool firstBeat = true; // used to seed rate array so we startup with reasonable BPM
volatile bool secondBeat = false; // used to seed rate array so we startup with reasonable BPM

volatile int BPM = 0; // int that holds raw Analog in 0. updated every 2mS
volatile int Signal; // holds the incoming raw data
volatile int IBI = 600; // int that holds the time interval between beats! Must be seeded!
volatile bool Pulse = false; // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile bool QS = false; // becomes true when finds a beat.