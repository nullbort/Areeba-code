#include <avr/pgmspace.h>
#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// Pin setup for 5 LEDs, 1 buzzer, and 1 button
const uint8_t ledPins[5] = {0, 1, 2, 3, 4}; // LEDs connected to these pins
const uint8_t buzzerPin = 1; // Shared with one LED (note: resource sharing)
const uint8_t buttonPin = 5; // Button connected here

// Songs stored in memory in a ringtone-like format (RTTTL)
const char tune1[] PROGMEM = 
  "ShonarBangla:d=4,o=5,b=100:"
  "g,e,f,g,a,b,2c6,8b,8a,g,f,e,d,2e,"
  "4g,4a,4b,2c6,8b,a,g,f,e,d,c,2b,"
  "4a,4g,8f,8e,8d,2c,4d,4e,"
  "g,e,f,g,a,b,2c6,4b,4a,4g,4f,2e";

const char tune2[] PROGMEM = 
  "BanglayGaan:d=4,o=5,b=112:"
  "e,e,f,g,g,a,2b,a,g,f,e,d,e,p,"
  "4c6,4b,4a,2g,8a,b,c6,4b,a,8g,f,e,"
  "4d,4e,2f,"
  "e,f,g,a,b,2c6,4b,4a,2g";

const char tune3[] PROGMEM = 
  "EkusherGaan:d=4,o=5,b=95:"
  "e,e,g,g,a,a,b,2a,g,f,e,d,c,d,"
  "4e,4f,4g,2a,8b,c6,d6,4c6,b,a,g,"
  "4f,4e,2d,"
  "e,g,a,b,2c6,4b,a,g,f,e,d";

const char tune4[] PROGMEM = 
  "OCanada:d=4,o=5,b=120:"
  "g,4c6,4e6,8d6,8c6,4b,4a,4g,2e,"
  "g,4c6,4e6,8d6,8c6,4b,4a,2g,"
  "c6,c6,b,4a,4g,4f,4e,2d,"
  "g,8a,b,c6,2d6,4c6,4b,4a,2g,"
  "4c6,4d6,4e6,4f6,4g6,2f6";

const char tune5[] PROGMEM = 
  "ShonarKonna:d=4,o=5,b=100:"
  "e,e,g,g,a,a,b,2c6,4b,a,g,f,e,"
  "d,d,e,f,g,a,b,c6,2b,4a,4g,4f,4e,"
  "e,g,a,b,c6,d6,e6,f6,g6,2a6";

// List of all tunes
const char* const tunes[] PROGMEM = {tune1, tune2, tune3, tune4, tune5};
const uint8_t tuneCount = sizeof(tunes) / sizeof(tunes[0]);

// For button press timing and behavior
unsigned long lastButtonDown = 0;
bool buttonHeld = false;
uint8_t nextTuneIndex = 0;
const unsigned long debounceDelay = 50;
const unsigned long longPressThreshold = 1500;

// Setup function runs once
void setup() {
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
    analogWrite(ledPins[i], 0);
  }
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  GIMSK |= (1 << PCIE);
  PCMSK |= (1 << PCINT5);
  sei();

  randomSeed(analogRead(0));
}

// Main loop
void loop() {
  static bool wasPressed = false;
  bool isPressed = (digitalRead(buttonPin) == LOW);
  unsigned long now = millis();

  if (isPressed && !wasPressed) {
    lastButtonDown = now;
    buttonHeld = false;
  }

  if (!isPressed && wasPressed) {
    unsigned long pressDuration = now - lastButtonDown;
    if (pressDuration > debounceDelay && pressDuration < longPressThreshold) {
      uint8_t index = random(tuneCount);
      playRTTTL(readProgmemTune(index));
    } else if (pressDuration >= longPressThreshold) {
      playRTTTL(readProgmemTune(nextTuneIndex));
      nextTuneIndex = (nextTuneIndex + 1) % tuneCount;
    }
  }

  wasPressed = isPressed;
  enterSleep();
}

// Sleep mode
void enterSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_bod_disable();
  sleep_cpu();
  sleep_disable();
}

// Wake from button interrupt
ISR(PCINT0_vect) {}

// Read a tune from program memory
const char* readProgmemTune(uint8_t index) {
  static char buffer[128];
  strcpy_P(buffer, (PGM_P)pgm_read_word(&(tunes[index])));
  return buffer;
}

// Play RTTTL
void playRTTTL(const char *p) {
  while (*p != ':') p++; p++;
  while (*p != ':') p++; p++;

  while (*p) {
    int duration = 0;
    while (isdigit(*p)) duration = duration * 10 + (*p++ - '0');
    if (!duration) duration = 4;

    int note = 0;
    switch (*p++) {
      case 'c': note = 1; break;
      case 'd': note = 3; break;
      case 'e': note = 5; break;
      case 'f': note = 6; break;
      case 'g': note = 8; break;
      case 'a': note = 10; break;
      case 'b': note = 12; break;
      default: note = 0;
    }

    if (*p == '#') { note++; p++; }
    bool dotted = false;
    if (*p == '.') { dotted = true; p++; }

    int octave = 6;
    if (isdigit(*p)) octave = *p++ - '0';
    if (*p == ',') p++;

    int wholenote = (60000 * 4) / 160;
    int noteDuration = wholenote / duration;
    if (dotted) noteDuration += noteDuration / 2;

    int freq = noteFreq(note, octave);
    uint8_t ledIndex = getLedIndex(freq);

    if (note) tone(buzzerPin, freq, noteDuration);
    beatPulse(ledIndex, noteDuration);

    delay(noteDuration * 0.2);
    noTone(buzzerPin);
  }

  clearLeds();
}

// LED fade effect
void beatPulse(uint8_t ledIndex, int duration) {
  const int steps = 20;
  int halfDur = duration / 2;

  for (int i = 0; i < steps; i++) {
    int val = map(i, 0, steps - 1, 0, 255);
    analogWrite(ledPins[ledIndex], val);
    delay(halfDur / steps);
  }

  for (int i = 0; i < steps; i++) {
    int val = map(i, 0, steps - 1, 255, 0);
    analogWrite(ledPins[ledIndex], val);
    delay(halfDur / steps);
  }

  analogWrite(ledPins[ledIndex], 0);
}

// Choose LED based on note frequency
uint8_t getLedIndex(int freq) {
  if (freq <= 300) return 0;
  if (freq <= 450) return 1;
  if (freq <= 600) return 2;
  if (freq <= 800) return 3;
  return 4;
}

// Convert note/octave to frequency
int noteFreq(int note, int octave) {
  static const float A4 = 440.0;
  int n = (octave - 4) * 12 + note - 10;
  return (int)(A4 * pow(2.0, n / 12.0));
}

// Turn off all LEDs
void clearLeds() {
  for (uint8_t i = 0; i < 5; i++) {
    analogWrite(ledPins[i], 0);
  }
}
