#include <rtttl.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// Define pins for LEDs, buzzer, and button
const uint8_t ledPins[3] = {0, 1, 3};         // 3 LEDs connected to these pins
const uint8_t buzzerPin = 4;                  // Buzzer connected to pin 1
const uint8_t buttonPin = 2;                  // Button connected to pin 4

// Songs stored as RTTTL strings in program memory (PROGMEM saves RAM)
const char tune1[] PROGMEM = "ShonarBangla:d=4,o=5,b=140:2g,a,a,4g,a#,2b,2c6,d6,2c6,b,a,g,2g,2f,2e,d,e,f,2g,g,a,b,c6,2b,a,g,a,b,2c6,2d6";
const char tune2[] PROGMEM = "BanglayGaan:d=4,o=5,b=150:c,d,e,f,g,2g,a,a,a#,b,2c6,b,a,g,f,2f,e,d,c,2c,d,e,f,g,a,2g,f,e,d,c,2c";
const char tune3[] PROGMEM = "EkusherGaan:d=4,o=5,b=135:d,e,f,g,a,b,c6,2c6,b,a,g,f,e,d,e,f,g,a,2b,2c6,b,a,g,f,e,d,c,2d";
const char tune4[] PROGMEM = "Canada:d=4,o=5,b=130:2g,a#,8p,8a#,2d#,p,f,g,g#,a#,c6,2f,2p,2g,a,8p,8a,2a#,p,c6,2d6,2c6,1a#";
const char tune5[] PROGMEM = "ShonarKonna:d=4,o=5,b=140:g,g,a,a,b,b,c6,d6,e6,d6,c6,b,a,g,f,e,d,c,b,a,g,2f,e,f,g,a,b,c6,d6,e6,f6,g6,a6,2b6";
const char tune6[] PROGMEM = "TwinkleT:d=4,o=5,b=80:32p,8c,8c,8g,8g,8a,8a,g,8f,8f,8e,8e,8d,8d,c,8g,8g,8f,8f,8e,8e,d,8g,8g,8f,8f,8e,8e,d,8c,8c,8g,8g,8a,8a,g";



// Array holding all the tunes for easy access
const char* const tunes[] PROGMEM = {tune1, tune2, tune3, tune4, tune5, tune6};
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

        }

  clearLeds();  // Turn off all LEDs at the end of the song
}


// Turn off all LEDs by setting brightness to zero
void clearLeds() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], 0);
  }
}
