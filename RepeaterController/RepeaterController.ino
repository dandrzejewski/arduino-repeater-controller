#include "morse.h"
/*

An Arduino-based repeater controller.

Author: David Andrzejewski


*/


// Configure here.
const int pinPtt = 2;
const int pinCor = 3;
const int pinPttLed = 13;

// Default is to have PTT pulled to ground when engaged, and open when disengaged.
const int pttEngaged = LOW;
const int pttDisengaged = HIGH;

// Timeout timer values
const unsigned long timeoutTimer = 50000;


// How often do we need to ID? A good value would probably be about 9.5 minutes.
const unsigned long idTimerInterval = 10000;

// ***** You probably do not need to touch anything below *****

// Track the last keyup time
unsigned long lastPttKeyupTime = 0;

// Track the ID timer
unsigned long idTimerStartTime = 0;

unsigned long timeoutTimerStartTime = 0;

int currentPttStatus = pttDisengaged;
int pttStatusBeforeId = pttDisengaged;
int currentCorStatus = LOW;
boolean timedOut = false;
boolean idInProgress = false;

// Morse code ID values
/*
According to http://www.nu-ware.com/NuCode%20Help/index.html?morse_code_structure_and_timing_.htm,

Dash Length = Dot length * 3
Pause between elements = dot length
Pause between characters = dot length * 3
Pause between words = dot length * 7

Speed (wpm = 2.4 * dots per second

*/

SpeakerMorseSender morse(4, 880, -1, 24);

void setup() {
  
  Serial.begin(9600);
  
  // Set up the PTT pin as an output
  pinMode(pinPtt, OUTPUT);

  // Ensure we start with the PTT pin high (off).
  digitalWrite(pinPtt, pttDisengaged);

  // Set up the COR pin as an input
  pinMode(pinCor, INPUT);

  pinMode(pinPttLed, OUTPUT);
  digitalWrite(pinPttLed, LOW);

  morse.setup();
  morse.setMessage(String("kd8twg/r"));

}

void loop() {
  if (timedOut) {
    if (digitalRead(pinCor) == LOW) {
      stopTimeoutTimer();
    } else {
      return;
    }
  }

  // When COR goes high, PTT needs to engage.
  if (digitalRead(pinCor) == HIGH && currentPttStatus == pttDisengaged) {
    setPtt(pttEngaged, true);
  }

  // If we have an ID in progress, we need to keep PTT engaged.
  if (currentPttStatus == pttDisengaged && idInProgress) {
    setPtt(pttEngaged, false);
  }

  // When COR goes low, PTT needs to disengage (if no ID is in progress)
  if (digitalRead(pinCor) == LOW && currentPttStatus == pttEngaged && !idInProgress) {
    setPtt(pttDisengaged, false);
  }


  doIdIfNeeded();

  if (!idInProgress) {
    doTimeoutIfNeeded();
  }
  
}

// Sets the PTT pin and sets up timers related to PTT (ID and Timeout)
void setPtt(int argPttHighLow, boolean startIdTimer) {

  // If we are just starting PTT, start the timeout timer.
  if (argPttHighLow == pttEngaged && currentPttStatus == pttDisengaged) {
    startTimeoutTimer();
  }

  // If we are stopping PTT, shut off the timeout timer.
  else if (argPttHighLow == pttDisengaged && currentPttStatus == pttEngaged) {
    stopTimeoutTimer();
  }

  // Track current PTT status
  currentPttStatus = argPttHighLow;

  // Start the ID timer - this does not get stopped or reset until ID happens.
  if (argPttHighLow == pttEngaged && startIdTimer) {
    startIdTimerIfNeeded();
  }


  // Set the PTT pin - but don't touch it if an ID is in progress.
  
  if (!idInProgress) {
    digitalWrite(pinPtt, argPttHighLow);
    digitalWrite(pinPttLed, !argPttHighLow);
  }

}

// If the ID timer hasn't been started, start it.
void startIdTimerIfNeeded() {
  if (idTimerStartTime == 0 && currentPttStatus == pttEngaged) {
    Serial.println("Starting ID timer.");
    idTimerStartTime = millis();
  }
}

// If it's time to do an ID, do it.
void doIdIfNeeded() {
  if (!morse.continueSending()) {
    idInProgress = false;

    if (millis() - idTimerStartTime > idTimerInterval && idTimerStartTime != 0) {

      Serial.println("Starting morse ID...");

      idTimerStartTime = 0;
      
      if (currentPttStatus == pttEngaged) {
        startIdTimerIfNeeded();
      }

      setPtt(pttEngaged, false);
    
      delay(250);

      idInProgress = true;

      morse.startSending();

    }
  }

}

// Start the timeout timer.
void startTimeoutTimer() {
  Serial.println("Starting timeout timer.");
  timeoutTimerStartTime = millis();
  timedOut = false;
}

// Stop the timeout timer.
void stopTimeoutTimer() {
  Serial.println("Stopping timeout timer.");
  timeoutTimerStartTime = 0;
  timedOut = false;
}

// Time out if needed.
void doTimeoutIfNeeded() {
  if (currentPttStatus == pttEngaged && millis() - timeoutTimerStartTime > timeoutTimer) {
    Serial.println("Timed out.");
    setPtt(pttDisengaged, false);
    timedOut = true;
  }
}
