/*
  Areeba

  by Chris Ritchie - 06/14/2025
*/

#include <rtttl.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// Define pins for LEDs, buzzer, and button
const uint8_t ledPins[3] = {0, 1, 3};         // 3 LEDs connected to these pins
const uint8_t buzzerPin = 4;                  // Buzzer connected to pin 1
const uint8_t buttonPin = 2;                  // Button connected to pin 4

// Songs stored as RTTTL strings in program memory (PROGMEM saves RAM)
const char tune1[] PROGMEM = "TwinkleT:d=4,o=5,b=80:32p,8c,8c,8g,8g,8a,8a,g,8f,8f,8e,8e,8d,8d,c,8g,8g,8f,8f,8e,8e,d,8g,8g,8f,8f,8e,8e,d,8c,8c,8g,8g,8a,8a,g";
const char tune2[] PROGMEM = "SuperMar:d=4,o=5,b=125:32p,8g,16c,8e,8g.,16c,8e,16g,16c,16e,16g,8b,a,8p,16c,8g,16c,8e,8g.,16c,8e,16g,16c#,16e,16g,8b,a,8p,16b,8c6,16b,8c6,8a.,16c6,8b,16a,8g,16f#,8g,8e.,16c,8d,16e,8f,16e,8f,8b.4,16e,8d.,c";
const char tune3[] PROGMEM = "PopGoesT:d=4,o=6,b=200:8g5,c,8c,d,8d,8e,8g,8e,c,8g5,c,8c,d,8d,e.,c,8g5,c,8c,d,8d,8e,8g,8e,c.,a.,d,8f,e.,c.,c,8c,a5,8c,8b5,8d,8b5,g.5,c,8c,a5,8c,b.5,g.5,f5,8e5,f5,8g5,a5,8b5,c,8d,g.,d,8f,e.,c.";
const char tune4[] PROGMEM = "BlackAdd:d=16,o=5,b=112:d6,p,4g.,d,p,g,p,d,p,g,p,b,p,2c#.6,p,a,p,4c.6,a,p,8c6,b,p,a,p,g,p,g,32p.,32g.,32a.,32g.,32a.,32g.,32a.,32g.,32a.,32g.,32a.,32g.,32a.,32g.,32a.,8f#,8p,d,p,4g.,d,p,g,p,d,p,g,p,b,p,4c#.6,a,p,c6,p,b,p,a,p,g,p,g,p,d,g,a,p,g,a,8d.6,4d6";
const char tune5[] PROGMEM = "RockABye:d=4,o=5,b=90:c,8d#,c6,2a#,g#,c,d#,g#,2g.,c#.,8d#,8c#.6,2c6,a#,a#,g#,f,2d#,8p,c.,8d#,c6,2a#,g#,c,d#,g#,2g,f,d#,g#,c#6,c6,g#.,a#,f,g#,2g#";
const char tune6[] PROGMEM = "I'mALitt:d=4,o=5,b=112:8c,8d,8e,8f,g,c6,a,c6,g,8c,f,8f,8g,e,c,d,8d,8d,2c,8p,8c,8d,8e,8f,g,c6,a,c6,g,8p,c6,a,g,8c,8d,e,d,c";
const char tune7[] PROGMEM = "Friends:d=16,o=5,b=180:4c#6,8c#6,8b,8a,4g,4g,8a,4b,4a,4p,4c#6,8c#6,8b,8a,4g,4g,8a,4b,4a,4c#6,4c#6,8c#6,8b,8a,4g,4g,8a,4b,4a,4p,4c#6,8c#6,8b,8a,4g,4g,8a,4b,4a";
const char tune8[] PROGMEM = "HeartOfA:d=4,o=5,b=155:8c6,8d6,8c6,8g,16c6,2p,p,8c6,8d6,8f6,8d6,16c6,8p,16p,8c6,8d6,8c6,8g,16a#,2p,p,16a#6,8p,16p,8a6,8f6,16d6,8p,16p,8c6,8d6,8c6,8a#,16c6,2p,p,8c6,8d6,8f6,8d6,16c6,8p,16p,8c6,8d6,8c6,8a#,16g";
const char tune9[] PROGMEM = "WakeMeUp:d=8,o=6,b=125:e,16g,p,16a,4p,16p,16e,g.,a,16e,p,16c,p,16c,d,16e,p,f.,16e,d,16c,e.,g,16e,p,16c,p,16c,e,16g,16p.,4a,p,16e,g.,a,16e,p,16c,p,16c,p,16c,p,16p,e.,c,d.,16e,d,d.,c.";
const char tune10[] PROGMEM = "Canada:d=4,o=5,b=130:2g,a#,8p,8a#,2d#,p,f,g,g#,a#,c6,2f,2p,2g,a,8p,8a,2a#,p,c6,2d6,2c6,1a#";
const char tune11[] PROGMEM = "Somewher:d=16,o=6,b=35:8g#5,8g#,g,32d#,32f,g,g#,8g#5,8f,4d#,8f5,8c#,c,32g#5,32a#5,c,c#,a#5,32g5,32g#5,a#5,c,8g#5";
const char tune12[] PROGMEM = "we-rock:d=4,o=6,b=125:8f5,8f5,4g#.5,8f5,4f5,2g#5,8f5,8g#5,4c#6,4c.6,8a#5,4a#5,4g#5,8d#5,8f5,4f#5,4d#5,8d#5,8f5,2f#5,8d#5,8f#5,8c6,8a#5,4g#5,4c6,2c#6,8c#5,8c#5,2c#6,8a#5,8f#5,2g#5,8f5,8c#5,4f#5,4g#5,4a#5,2g#5,8c#5,8c#5,2c#6,8a#5,8f#5,2g#5,8f5,8c#5,8f#5,16g#5,16f#5,4f5,4d#5,2c#5";
const char tune13[] PROGMEM = "FrostyTh:d=4,o=5,b=140:2g,e.,8f,g,2c6,8b,8c6,d6,c6,b,a,2g,p,8b,8c6,d6,c6,b,8a,8a,g,c6,e,8f,8a,g,f,e,f,2g";
const char tune14[] PROGMEM = "Ferouz:d=4,o=6,b=180:1b5,8a.5,8b.5,1c,8b.5,8c.,8a.5,1b5,p,8a.5,8b.5,1c,8b.5,8d.,8c.,1b5,8a.5,8b.5,8c.,8b.5,1f_5,g.5,8a.5,8g.5,1d_5,f_.5,8g.5,e.5";
const char tune15[] PROGMEM = "YommaTho:d=4,o=6,b=160:g5,g5,g5,2b5,2b.5,8c,8b5,a5,g5,2a5,g5,f5,8g5,8f5,1f5,f5,f5,g5,2a5,2a5,8b5,8a5,g5,f5,e5,2g5,f5,e5,1e5,g5,g5,a5,2b5,2b5,a5,a5,d,c,1b5,b5,8c,8b5,a5,g5,8a5,8g5,8f5,8b5,2a5,a5";
const char tune16[] PROGMEM = "Brandenb:d=4,o=6,b=125:16g,16f_,8g,16d,16c,8d,16g,16f_,8g,16b5,16a5,8b5,16g,16f_,8g,16g5,16a5,8b5,8c_,16d,16c_,16d,16e,16d,16f_,16d,16g,16d,16c_,16d,16e,16d,16a,16d,16b,16d,16c_,16d,16e,16d,16c7,16d,16d7,8b,16a,16g,8a,16g,16f_,g";
const char tune17[] PROGMEM = "Children:d=4,o=6,b=80:2f,8g#,16g,2d#,8g#,16g,2c,8g#,16g,2g#5,32f5,32g5,32g#5,32c,2f,8g#,16g,2d#,32c#,32c,8c#,16c,g#.5,8g.5,8g#5,16c,8f.5";
const char tune18[] PROGMEM = "TeddyBea:d=32,o=6,b=45:g#5,16c#,e,d#,e,d#,16c#,e,16d#,e,c#,d#,e,16d#,e,8c#.,b5,16e,g#,f#,g#,f#,16e,g#,16f#,g#,e,f#,g#,16f#,g#,8e.,b,16c#7,b,16c#7,b,f#,g#,b,16g#,f#,16f#,e,16f#,e,b5,c#,e,16c#,b5,16c#,32e6,16b5,32e6,32c#6,32d#6,32e6,16b5,32g#6,8f#6,16p,8e6";

// Array holding all the tunes for easy access
const char* const tunes[] PROGMEM = {tune1, tune2, tune3, tune4, tune5, tune6, tune7, tune8, tune9, tune10, tune11, tune12, tune13, tune14, tune15, tune16, tune17, tune18};
const uint8_t tuneCount = sizeof(tunes) / sizeof(tunes[0]);  // Count of total tunes

// Variables for button press timing and state
unsigned long lastButtonDown = 0;             // Time when button was pressed
bool buttonHeld = false;                       // Whether button is being held
uint8_t nextTuneIndex = 0;                     // Index of next tune to play on long press
const unsigned long debounceDelay = 50;       // Time to ignore button bounce (milliseconds)
//const unsigned long longPressThreshold = 1500; // Time to consider press a long press (milliseconds)
const int octave = 0;
int playSong = 0;

Rtttl player;

void setup() {
  player.begin(buzzerPin);
  // Set each LED pin as output so we can turn them on/off
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(buzzerPin, OUTPUT);           // Buzzer pin set as output
  pinMode(buttonPin, INPUT_PULLUP);     // Button pin input with internal pull-up resistor enabled

  // Enable pin change interrupt for the button pin (to wake from sleep)
  GIMSK |= (1 << PCIE);     // Enable pin change interrupts
  PCMSK |= (1 << PCINT4);   // Enable interrupt for button pin (PCINT4)
  sei();                   // Enable global interrupts

  randomSeed(analogRead(0)); // Seed random number generator with analog noise for randomness
}

void enterLightSleep() {
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}

ISR(PCINT0_vect) {
  // Wake from sleep on pin change
}

void loop() {
  static bool wasPressed = false;                  // Keeps track of previous button state
  bool isPressed = (digitalRead(buttonPin) == LOW); // Button reads LOW when pressed
  bool playState = digitalRead(buttonPin);
  unsigned long now = millis();                     // Current time in milliseconds since startup

  if (playSong == tuneCount) playSong = 0;


  // Detect the moment button is pressed down
  if (isPressed && !wasPressed) {
    lastButtonDown = now;   // Record time button was pressed
    buttonHeld = false;     // Reset held flag
  }

  // Detect when button is released
  if (!isPressed && wasPressed) {
    unsigned long pressDuration = now - lastButtonDown; // Calculate how long button was held

    // If it was a short press (debounce time < press < long press time)
    if (pressDuration > debounceDelay) {
      playRTTTL(playSong);               // Play a random tune
      playSong++;
    }
  }

  wasPressed = isPressed;  // Save button state for next loop iteration
  
/*
      if (playState == LOW) {
        if (playSong > 5) playSong = 0;
      playRTTTL(playSong);               // Play a random tune
      playSong++;
    }
    */
  enterLightSleep();            // Put microcontroller into sleep to save power until button pressed again
}

// Function to play a RTTTL tune passed as a string
void playRTTTL(uint8_t playSong) {

// Try player.play_P(tunes[playSong], octave);

        switch (playSong)
        {
        case 0:
            player.play_P(tune1, octave);
            break;
        case 1:
            player.play_P(tune2, octave);
            break;
        case 2:
            player.play_P(tune3, octave);
            break;
        case 3:
            player.play_P(tune4, octave);
            break;
        case 4:
            player.play_P(tune5, octave);
            break;
        case 5:
            player.play_P(tune6, octave);
            break;
        case 6:
            player.play_P(tune7, octave);
            break;
        case 7:
            player.play_P(tune8, octave);
            break;
        case 8:
            player.play_P(tune9, octave);
            break;
        case 9:
            player.play_P(tune10, octave);
            break;
        case 10:
            player.play_P(tune11, octave);
            break;
        case 11:
            player.play_P(tune12, octave);
            break;
        case 12:
            player.play_P(tune13, octave);
            break;
        case 13:
            player.play_P(tune14, octave);
            break;
        case 14:
            player.play_P(tune15, octave);
            break;
        case 15:
            player.play_P(tune16, octave);
            break;
        case 16:
            player.play_P(tune17, octave);
            break;
        case 17:
            player.play_P(tune18, octave);
            break;
        }

  clearLeds();  // Turn off all LEDs at the end of the song
}


// Turn off all LEDs by setting brightness to zero
void clearLeds() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], 0);
  }
}
