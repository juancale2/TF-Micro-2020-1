#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "sensores.h"
#include "LCD.h"

unsigned int getSignal(char sensor) {
    ADC_CONFIG(sensor);
    unsigned int signal = ADC_CONVERTIR();
    return signal;
}

void samplePulse(void) {
    Signal = getSignal(PULSIMETRO);

    sampleCounter += 2; // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime; // monitor the time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave
    if (Signal < thresh && N > (IBI / 5)*3) { // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal < T) { // T is the trough
            T = Signal; // keep track of lowest point in pulse wave
        }
    }

    if (Signal > thresh && Signal > P) { // thresh condition helps avoid noise
        P = Signal; // P is the peak
    } // keep track of highest point in pulse wave

    //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
    // signal surges up in value every time there is a pulse
    if (N > 250) { // avoid high frequency noise
        if ((Signal > thresh) && (Pulse == false) && (N > (IBI / 5)*3)) {
            Pulse = true; // set the Pulse flag when we think there is a pulse
            IBI = sampleCounter - lastBeatTime; // measure time between beats in mS
            lastBeatTime = sampleCounter; // keep track of time for next pulse

            if (secondBeat) { // if this is the second beat, if secondBeat == TRUE
                secondBeat = false; // clear secondBeat flag
                int i;
                for (i = 0; i <= 9; i++) { // seed the running total to get a realisitic BPM at startup
                    rate[i] = IBI;
                }
            }

            if (firstBeat) { // if it's the first time we found a beat, if firstBeat == TRUE
                firstBeat = false; // clear firstBeat flag
                secondBeat = true; // set the second beat flag
                //pulse_tmr_handle = bsp_harmony_start_tmr_cb_periodic(PULSE_CHECK_TIME_INTERVAL, 0, pulse_read_cb); // enable interrupts again
                return; // IBI value is unreliable so discard it
            }

            // keep a running total of the last 10 IBI values
            unsigned int runningTotal = 0; // clear the runningTotal variable
            int i;
            for (i = 0; i <= 8; i++) { // shift data in the rate array
                rate[i] = rate[i + 1]; // and drop the oldest IBI value
                runningTotal += rate[i]; // add up the 9 oldest IBI values
            }

            rate[9] = IBI; // add the latest IBI to the rate array
            runningTotal += rate[9]; // add the latest IBI to runningTotal
            runningTotal /= 10; // average the last 10 IBI values
            BPM = 60000 / runningTotal; // how many beats can fit into a minute? that's BPM!
            QS = true; // set Quantified Self flag
            // QS FLAG IS NOT CLEARED INSIDE THIS ISR
        }
    }
if (Signal < thresh && Pulse == true) { // when the values are going down, the beat is over
        Pulse = false; // reset the Pulse flag so we can do it again
        amp = P - T; // get amplitude of the pulse wave
        thresh = amp / 2 + T; // set thresh at 50% of the amplitude
        P = thresh; // reset these for next time
        T = thresh;
    }

    if (N > 2500) { // if 2.5 seconds go by without a beat
        BPM = 0;
        thresh = 530; // set thresh default
        P = 512; // set P default
        T = 512; // set T default
        lastBeatTime = sampleCounter; // bring the lastBeatTime up to date
        firstBeat = true; // set these to avoid noise
        secondBeat = false; // when we get the heartbeat back
    }
    return;
}

unsigned int getOxi(void) {
    unsigned int Oxi_bin = getSignal(OXIMETRO);
    //Escalando la entrada
    //Oxi_bin ->   0 - 1023
    //Oxi_real ->  0% - 100%
    unsigned int Oxi_real = Oxi_bin*0.097752;
    return Oxi_real;
}

float getTemp(void) {
    int Temp_bin = getSignal(TERMOMETRO);
    //Escalando la entrada
    //Temp_bin ->   0 - 1023
    //Temp_real ->  30°C - 50°C
    float Temp_real = (float)Temp_bin*500/1023;
    return Temp_real;
}
