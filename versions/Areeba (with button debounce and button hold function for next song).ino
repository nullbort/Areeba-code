/*
✅ New Features

    Button debounce using software delay.

    Beat-synced LED pulsing (LEDs pulse gently during each note).

    Random RTTTL tune selection on button press.

    Smooth LED fade-in/fade-out tied to note duration.
    
    🟢 Long-press detection

    Short press → Play a random song

    Long press (hold ≥1.5s) → Play the next song in sequence
    
| Pin | Component                                    |
| --- | -------------------------------------------- |
| 0–4 | 5 LEDs (PWM-capable if possible)             |
| 1   | Piezo buzzer                                 |
| 5   | Push button to GND (internal pullup enabled) |


🎉 Result

    Press the button → a random song plays.

    Each note triggers a single LED pulse mapped to its pitch.

    Pulses fade in/out smoothly, creating a rhythmic visual synced to music.

    Button presses are debounced to avoid retriggering.
    
    🧪 How It Works

    Press the button quickly → random song

    Hold the button ≥1.5 seconds → next song in order

    Button is debounced to prevent glitches

    LEDs fade in/out per note, based on pitch
*/


#include <avr/pgmspace.h>
#include <Arduino.h>

// Pin setup
const uint8_t ledPins[5] = {0, 1, 2, 3, 4};
const uint8_t buzzerPin = 1;
const uint8_t buttonPin = 5;
// Longer RTTTL melodies (approximate and simplified for ATtiny85)

// Bangladesh: Amar Shonar Bangla (extended)
const char tune1[] PROGMEM = 
  "ShonarBangla:d=4,o=5,b=100:"
  "g,e,f,g,a,b,2c6,8b,8a,g,f,e,d,2e,"
  "4g,4a,4b,2c6,8b,a,g,f,e,d,c,2b,"
  "4a,4g,8f,8e,8d,2c,4d,4e";

// Bangladesh: Ami Banglay Gaan Gai (extended)
const char tune2[] PROGMEM = 
  "BanglayGaan:d=4,o=5,b=112:"
  "e,e,f,g,g,a,2b,a,g,f,e,d,e,p,"
  "4c6,4b,4a,2g,8a,b,c6,4b,a,8g,f,e,"
  "4d,4e,2f";

// Bangladesh: Ekusher Gaan (Amar Bhaiyer Rokte Rangano) (extended)
const char tune3[] PROGMEM = 
  "EkusherGaan:d=4,o=5,b=95:"
  "e,e,g,g,a,a,b,2a,g,f,e,d,c,d,"
  "4e,4f,4g,2a,8b,c6,d6,4c6,b,a,g,"
  "4f,4e,2d";

// Canada: O Canada (simplified)
const char tune4[] PROGMEM = 
  "OCanada:d=4,o=5,b=120:"
  "g,4c6,4e6,8d6,8c6,4b,4a,4g,2e,"
  "g,4c6,4e6,8d6,8c6,4b,4a,2g,"
  "c6,c6,b,4a,4g,4f,4e,2d,"
  "g,8a,b,c6,2d6,4c6,4b,4a,2g";

const char* const tunes[] PROGMEM = {tune1, tune2, tune3, tune4};
const uint8_t tuneCount = sizeof(tunes) / sizeof(tunes[0]);



// State tracking
unsigned long lastButtonDown = 0;
bool buttonHeld = false;
uint8_t nextTuneIndex = 0;

const unsigned long debounceDelay = 50;
const unsigned long longPressThreshold = 1500; // 1.5 seconds

void setup() {
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
    analogWrite(ledPins[i], 0);
  }

  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  randomSeed(analogRead(0));
}

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
      // Short press → Random tune
      uint8_t index = random(tuneCount);
      playRTTTL(readProgmemTune(index));
    } else if (pressDuration >= longPressThreshold) {
      // Long press → Next tune in sequence
      playRTTTL(readProgmemTune(nextTuneIndex));
      nextTuneIndex = (nextTuneIndex + 1) % tuneCount;
    }
  }

  wasPressed = isPressed;
}

const char* readProgmemTune(uint8_t index) {
  static char buffer[128];
  strcpy_P(buffer, (PGM_P)pgm_read_word(&(tunes[index])));
  return buffer;
}

// --- RTTTL Playback with Beat-Pulse LEDs ---
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

    delay(noteDuration * 0.2);  // short pause
    noTone(buzzerPin);
  }

  clearLeds();
}

void beatPulse(uint8_t ledIndex, int duration) {
  const int steps = 20;
  int halfDur = duration / 2;

  // Fade in
  for (int i = 0; i < steps; i++) {
    int val = map(i, 0, steps - 1, 0, 255);
    analogWrite(ledPins[ledIndex], val);
    delay(halfDur / steps);
  }

  // Fade out
  for (int i = 0; i < steps; i++) {
    int val = map(i, 0, steps - 1, 255, 0);
    analogWrite(ledPins[ledIndex], val);
    delay(halfDur / steps);
  }

  analogWrite(ledPins[ledIndex], 0);
}

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

void clearLeds() {
  for (uint8_t i = 0; i < 5; i++) {
    analogWrite(ledPins[i], 0);
  }
}
