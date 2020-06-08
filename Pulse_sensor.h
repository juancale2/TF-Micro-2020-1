#include <xc.h>

typedef unsigned char bool;
#define true    1
#define false   0


void samplePulse(void);
int getSignal();

// Volatile Variables, used in the interrupt service routine!
int BPM;                   // int that holds raw Analog in 0. updated every 2mS
int Signal;                // holds the incoming raw data
int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
bool Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 

int rate[10];                      // array to hold last ten IBI values
unsigned long sampleCounter = 0;          // used to determine pulse timing
unsigned long lastBeatTime = 0;           // used to find IBI
int P = 512;                      // used to find peak in pulse wave, seeded
int T = 512;                     // used to find trough in pulse wave, seeded
int thresh = 525;                // used to find instant moment of heart beat, seeded
int amp = 100;                   // used to hold amplitude of pulse waveform, seeded
bool firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
bool secondBeat = false;      // used to seed rate array so we startup with reasonable BPM

