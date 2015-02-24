/*

An Arduino-based repeater controller.

Author: David Andrzejewski


*/


const int pinPtt = 2;
const int pinCor = 3;

int currentPttStatus = LOW;
int currentCorStatus = LOW;

// How often do we need to ID? A good value would probably be about 9.5 minutes.
const unsigned long idTimerInterval = 30000;

// Track the last keyup time
unsigned long lastPttKeyupTime = 0;

// Track the ID timer
unsigned long idTimerStartTime = 0;
boolean pendingId = false;

// Timeout timer values
const unsigned long timeoutTimer = 30000;
unsigned long timeoutTimerStartTime = 0;

// Morse code ID values
/* 
According to http://www.nu-ware.com/NuCode%20Help/index.html?morse_code_structure_and_timing_.htm,

Dash Length = Dot length * 3
Pause between elements = dot length
Pause between characters = dot length * 3
Pause between words = dot length * 7

Speed (wpm = 2.4 * dots per second

*/ 

// All lengths in millis - 100ms = 24wpm
const int dotLength = 100;
const int pauseBetweenElements = dotLength;
const int pauseBetweenCharacters = dotLength * 3;
const int pauseBetweenWords = dotLength * 7;

void setup() {
  // Set up the PTT pin as an output
  pinMode(pinPtt, OUTPUT);

  // Ensure we start with the PTT pin low.
  digitalWrite(pinPtt, LOW);

  // Set up the COR pin as an input
  pinMode(pinCor, INPUT);
  
}

void loop() {
  // When COR goes high, PTT needs to go high.
  if (digitalRead(pinCor) == HIGH && currentPttStatus == LOW) {
    setPtt(HIGH);
  }

  // When COR goes low, PTT needs to go low.
  if (digitalRead(pinCor) == LOW && currentPttStatus == HIGH) {
    setPtt(LOW);
  }
  
  doIdIfNeeded();
  
  doTimeoutIfNeeded();
  
}

// Sets the PTT pin and sets up timers related to PTT (ID and Timeout)
void setPtt(int argPttHighLow) {
  
  // If we are just starting PTT, start the timeout timer.
  if (argPttHighLow == HIGH && currentPttStatus == LOW) {
    startTimeoutTimer();
  }
  
  // If we are stopping PTT, shut off the timtout timer.
  else if (argPttHighLow == LOW && currentPttStatus == HIGH) {
    stopTimeoutTimer();
  }
  
  // Start the ID timer - this does not get stopped or reset until ID happens.
  if (argPttHighLow == HIGH) {
    startIdTimerIfNeeded();
  }

  // Track current PTT status
  currentPttStatus = argPttHighLow;

  // Set the PTT pin.
  digitalWrite(pinPtt, argPttHighLow);
  
}

// If the ID timer hasn't been started, start it.
void startIdTimerIfNeeded() {
  if (idTimerStartTime == 0 && digitalRead(pinPtt) == HIGH) {
    idTimerStartTime = millis();
  }
}

// If it's time to do an ID, do it.
void doIdIfNeeded() {
  if (millis() - idTimerStartTime > idTimerInterval) {
    // Do the ID
    
    // Reset the timer
    idTimerStartTime = 0;

  }
}

// Start the timeout timer.
void startTimeoutTimer() {
  timeoutTimerStartTime = millis();
}

// Stop the timeout timer.
void stopTimeoutTimer() {
  timeoutTimerStartTime = 0;
}

// Time out if needed.
void doTimeoutIfNeeded() {
  if (millis() - timeoutTimerStartTime > timeoutTimer) {
    setPtt(LOW);
  }
}
