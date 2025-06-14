#include <avr/pgmspace.h>       // For storing data in program memory (saves RAM)
#include <Arduino.h>            // Basic Arduino functions
#include <avr/sleep.h>          // Sleep mode functions to save power
#include <avr/interrupt.h>      // Interrupt handling functions

// Define pins for LEDs, buzzer, and button
const uint8_t ledPins[3] = {0, 2, 3};         // 3 LEDs connected to these pins
const uint8_t buzzerPin = 1;                  // Buzzer connected to pin 1
const uint8_t buttonPin = 4;                  // Button moved to pin 4

// Songs stored as RTTTL strings in program memory (PROGMEM saves RAM)
const char tune1[] PROGMEM = "ShonarBangla:d=4,o=5,b=100:g,e,f,g,a,b,2c6,8b,8a,g,f,e,d,2e,4g,4a,4b,2c6,8b,a,g,f,e,d,c,2b,4a,4g,8f,8e,8d,2c,4d,4e,g,e,f,g,a,b,2c6,4b,4a,4g,4f,2e";
const char tune2[] PROGMEM = "BanglayGaan:d=4,o=5,b=112:e,e,f,g,g,a,2b,a,g,f,e,d,e,p,4c6,4b,4a,2g,8a,b,c6,4b,a,8g,f,e,4d,4e,2f,e,f,g,a,b,2c6,4b,4a,2g";
const char tune3[] PROGMEM = "EkusherGaan:d=4,o=5,b=95:e,e,g,g,a,a,b,2a,g,f,e,d,c,d,4e,4f,4g,2a,8b,c6,d6,4c6,b,a,g,4f,4e,2d,e,g,a,b,2c6,4b,a,g,f,e,d";
const char tune4[] PROGMEM = "OCanada:d=4,o=5,b=120:g,4c6,4e6,8d6,8c6,4b,4a,4g,2e,g,4c6,4e6,8d6,8c6,4b,4a,2g,c6,c6,b,4a,4g,4f,4e,2d,g,8a,b,c6,2d6,4c6,4b,4a,2g,4c6,4d6,4e6,4f6,4g6,2f6";
const char tune5[] PROGMEM = "ShonarKonna:d=4,o=5,b=100:e,e,g,g,a,a,b,2c6,4b,a,g,f,e,d,d,e,f,g,a,b,c6,2b,4a,4g,4f,4e,e,g,a,b,c6,d6,e6,f6,g6,2a6";

// Array holding all the tunes for easy access
const char* const tunes[] PROGMEM = {tune1, tune2, tune3, tune4, tune5};
const uint8_t tuneCount = sizeof(tunes) / sizeof(tunes[0]);  // Count of total tunes

// Variables for button press timing and state
unsigned long lastButtonDown = 0;             // Time when button was pressed
bool buttonHeld = false;                       // Whether button is being held
uint8_t nextTuneIndex = 0;                     // Index of next tune to play on long press
const unsigned long debounceDelay = 50;       // Time to ignore button bounce (milliseconds)
const unsigned long longPressThreshold = 1500; // Time to consider press a long press (milliseconds)

void setup() {
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

void loop() {
  static bool wasPressed = false;                  // Keeps track of previous button state
  bool isPressed = (digitalRead(buttonPin) == LOW); // Button reads LOW when pressed
  unsigned long now = millis();                     // Current time in milliseconds since startup

  // Detect the moment button is pressed down
  if (isPressed && !wasPressed) {
    lastButtonDown = now;   // Record time button was pressed
    buttonHeld = false;     // Reset held flag
  }

  // Detect when button is released
  if (!isPressed && wasPressed) {
    unsigned long pressDuration = now - lastButtonDown; // Calculate how long button was held

    // If it was a short press (debounce time < press < long press time)
    if (pressDuration > debounceDelay && pressDuration < longPressThreshold) {
      uint8_t index = random(tuneCount);               // Pick a random song index
      playRTTTL(readProgmemTune(index));               // Play a random tune

    // If it was a long press
    } else if (pressDuration >= longPressThreshold) {
      playRTTTL(readProgmemTune(nextTuneIndex));       // Play the next tune in order
      nextTuneIndex = (nextTuneIndex + 1) % tuneCount; // Update next tune index (loops around)
    }
  }

  wasPressed = isPressed;  // Save button state for next loop iteration
  enterSleep();            // Put microcontroller into sleep to save power until button pressed again
}

// Function to put the microcontroller to sleep to save battery
void enterSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Use deepest sleep mode
  sleep_enable();                       // Enable sleep mode
  sleep_bod_disable();                  // Disable brown-out detector for extra power saving
  sleep_cpu();                         // Actually go to sleep here, waits for interrupt to wake
  sleep_disable();                     // Disable sleep when wakes up
}

// Interrupt Service Routine (ISR) for pin change interrupt (wakes device but no action needed)
ISR(PCINT0_vect) {}

// Function to copy a tune from program memory into a buffer for playing
const char* readProgmemTune(uint8_t index) {
  static char buffer[128];                                 // Buffer to hold tune text
  strcpy_P(buffer, (PGM_P)pgm_read_word(&(tunes[index]))); // Copy tune from PROGMEM to buffer
  return buffer;                                           // Return pointer to buffer
}

// Function to play a RTTTL tune passed as a string
void playRTTTL(const char *p) {
  while (*p != ':') p++; p++;  // Skip tune name until first ':'
  while (*p != ':') p++; p++;  // Skip tune defaults until second ':'

  // Loop through the notes in the tune string
  while (*p) {
    int duration = 0;

    // Read duration number if present (e.g. 4 in d=4)
    while (isdigit(*p)) duration = duration * 10 + (*p++ - '0');
    if (!duration) duration = 4;  // Default duration if none specified

    // Determine note value based on character
    int note = 0;
    switch (*p++) {
      case 'c': note = 1; break;
      case 'd': note = 3; break;
      case 'e': note = 5; break;
      case 'f': note = 6; break;
      case 'g': note = 8; break;
      case 'a': note = 10; break;
      case 'b': note = 12; break;
      default: note = 0;             // Pause or unknown note
    }

    // Check for sharp (#) notes
    if (*p == '#') { note++; p++; }

    // Check for dotted notes (adds half the duration)
    bool dotted = false;
    if (*p == '.') { dotted = true; p++; }

    // Get octave number or use default
    int octave = 6;
    if (isdigit(*p)) octave = *p++ - '0';

    if (*p == ',') p++;  // Skip comma between notes

    // Calculate how long this note lasts in milliseconds
    int wholenote = (60000 * 4) / 160;  // Base length of whole note at tempo 160 bpm
    int noteDuration = wholenote / duration;
    if (dotted) noteDuration += noteDuration / 2;

    // Calculate frequency to play this note
    int freq = noteFreq(note, octave);

    // Choose LED to light based on frequency range
    uint8_t ledIndex = getLedIndex(freq);

    // Play note and pulse LED if not a pause
    if (note) tone(buzzerPin, freq, noteDuration);  // Start buzzer tone
    beatPulse(ledIndex, noteDuration);              // Pulse corresponding LED

    delay(noteDuration * 0.2);   // Brief pause between notes
    noTone(buzzerPin);           // Stop buzzer tone
  }

  clearLeds();  // Turn off all LEDs at the end of the song
}

// Smoothly fade LED on and off to match the beat duration
void beatPulse(uint8_t ledIndex, int duration) {
  for (int i = 0; i < 4; i++) analogWrite(ledPins[i], 0);  // Turn all LEDs off

  // Fade LED brightness up
  for (int i = 0; i < duration / 2; i += 5) {
    analogWrite(ledPins[ledIndex], i * 255 / (duration / 2));  // Increase brightness
    delay(5);
  }

  // Fade LED brightness down
  for (int i = duration / 2; i > 0; i -= 5) {
    analogWrite(ledPins[ledIndex], i * 255 / (duration / 2));  // Decrease brightness
    delay(5);
  }

  analogWrite(ledPins[ledIndex], 0);  // Ensure LED is off at the end
}

// Decide which LED to light based on note frequency
uint8_t getLedIndex(int freq) {
  if (freq <= 300) return 0;
  if (freq <= 450) return 1;
  if (freq <= 600) return 2;
  return 3;
}

// Calculate the frequency of a note given its note number and octave
int noteFreq(int note, int octave) {
  static const float A4 = 440.0;  // Frequency of note A4 (standard pitch)
  int n = (octave - 4) * 12 + note - 10; // Calculate semitone difference from A4
  return (int)(A4 * pow(2.0, n / 12.0)); // Calculate frequency using equal temperament formula
}

// Turn off all LEDs by setting brightness to zero
void clearLeds() {
  for (uint8_t i = 0; i < 4; i++) {
    analogWrite(ledPins[i], 0);
  }
}
