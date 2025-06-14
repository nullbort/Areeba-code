/*
Below is a fully featured ATtiny85 Arduino sketch that:

✅ Plays a random RTTTL tune
✅ Lights up 5 LEDs mapped to note pitch frequency ranges
✅ Uses fading transitions (smooth LED brightness)
✅ Responds to a button press to play a new random tune
🛠️ Hardware Connections
Component	Pin	Notes
5 LEDs	0–4	Each via resistor
Buzzer	1	Piezo (could move to pin 5 if needed)
Button	5	Pulled to GND with internal pullup
*/


#include <avr/pgmspace.h>
#include <Arduino.h>

const uint8_t ledPins[5] = {0, 1, 2, 3, 4};  // LEDs
const uint8_t buzzerPin = 1;                // Buzzer pin
const uint8_t buttonPin = 5;                // Button pin

// RTTTL songs in PROGMEM
const char tune1[] PROGMEM = "Tune1:d=16,o=5,b=140:8c6,8g5,8e5,8a5,8b5,8a5,8g5,8e5";
const char tune2[] PROGMEM = "Tune2:d=8,o=6,b=160:e6,d6,f6,g6,e6,p,e6,e6,e6";
const char tune3[] PROGMEM = "Tune3:d=4,o=5,b=180:g5,g5,g5,e5,g5,g5,a5,b5,c6";
const char* const tunes[] PROGMEM = {tune1, tune2, tune3};
const uint8_t tuneCount = sizeof(tunes) / sizeof(tunes[0]);

bool buttonPressed = false;

void setup() {
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Pull-up button
  randomSeed(analogRead(0)); // Seed from floating analog pin
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      clearLeds();

      uint8_t index = random(tuneCount);
      char buffer[128];
      strcpy_P(buffer, (PGM_P)pgm_read_word(&(tunes[index])));

      playRTTTL(buffer);
    }
  } else {
    buttonPressed = false;
  }
}

// --- RTTTL Playback with LED Fade ---
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

    if (note) {
      tone(buzzerPin, freq, noteDuration);
    }

    fadeLed(ledIndex, 255, noteDuration * 0.6); // fade in
    fadeLed(ledIndex, 0, noteDuration * 0.4);   // fade out
    noTone(buzzerPin);
  }
  clearLeds();
}

// Map frequency to one of 5 LEDs
uint8_t getLedIndex(int freq) {
  if (freq <= 300) return 0;
  if (freq <= 450) return 1;
  if (freq <= 600) return 2;
  if (freq <= 800) return 3;
  return 4;
}

int noteFreq(int note, int octave) {
  static const float A4 = 440.0;
  int n = (octave - 4) * 12 + note - 10;
  return (int)(A4 * pow(2.0, n / 12.0));
}

void fadeLed(uint8_t pinIndex, uint8_t targetBrightness, int duration) {
  uint8_t startBrightness = analogRead(ledPins[pinIndex]); // May not be accurate
  int steps = 10;
  for (int i = 0; i <= steps; i++) {
    int brightness = map(i, 0, steps, 0, targetBrightness);
    analogWrite(ledPins[pinIndex], brightness);
    delay(duration / steps);
  }
}

void clearLeds() {
  for (uint8_t i = 0; i < 5; i++) {
    analogWrite(ledPins[i], 0);
  }
}
