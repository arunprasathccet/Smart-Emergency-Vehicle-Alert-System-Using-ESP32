#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

/*
  Smart Emergency Vehicle Alert System
  ESP32 + 2 x MAX9814 + 128x64 OLED + Red/Green LEDs

  Connections:
  Left MAX9814 OUT  -> GPIO34
  Right MAX9814 OUT -> GPIO35
  OLED SDA          -> GPIO21
  OLED SCL          -> GPIO22
  Red LED           -> GPIO18 through 220 ohm resistor
  Green LED         -> GPIO19 through 220 ohm resistor

  OLED I2C address: 0x3C

  Detection uses:
  1. Audio RMS level
  2. Approximate frequency from zero crossings
  3. Multiple consecutive detection windows
  4. Two-microphone level comparison for direction

  IMPORTANT:
  Siren detection thresholds must be calibrated in your actual prototype.
*/

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int MIC_LEFT  = 34;
const int MIC_RIGHT = 35;

const int RED_LED   = 18;
const int GREEN_LED = 19;

const uint32_t SAMPLE_RATE = 8000;
const uint16_t SAMPLES_PER_WINDOW = 256;

// Adjustable siren frequency range
const float SIREN_MIN_HZ = 600.0;
const float SIREN_MAX_HZ = 1800.0;

// Adjust this after watching RMS values in Serial Monitor
const float MIN_RMS = 45.0;

const int DETECT_CONFIRM_COUNT = 4;
const int RELEASE_COUNT = 10;

const float DIRECTION_MARGIN = 1.20;

bool sirenDetected = false;
int goodCount = 0;
int badCount = 0;

enum Direction {
  DIR_FRONT,
  DIR_LEFT,
  DIR_RIGHT
};

Direction currentDirection = DIR_FRONT;

struct AudioInfo {
  float rms;
  float frequency;
};

AudioInfo analyzeMic(int pin) {
  float samples[SAMPLES_PER_WINDOW];
  double sum = 0.0;

  const uint32_t samplePeriodUs = 1000000UL / SAMPLE_RATE;
  uint32_t nextSample = micros();

  for (uint16_t i = 0; i < SAMPLES_PER_WINDOW; i++) {
    while ((int32_t)(micros() - nextSample) < 0) {
      // Wait for the next sample time
    }

    int raw = analogRead(pin);
    samples[i] = (float)raw;
    sum += samples[i];
    nextSample += samplePeriodUs;
  }

  float dc = (float)(sum / SAMPLES_PER_WINDOW);

  double sumSq = 0.0;
  int crossings = 0;
  const float crossingThreshold = 8.0;

  bool previousPositive = (samples[0] - dc) > crossingThreshold;

  for (uint16_t i = 0; i < SAMPLES_PER_WINDOW; i++) {
    float x = samples[i] - dc;
    sumSq += (double)x * (double)x;

    bool positive = previousPositive;

    if (x > crossingThreshold) {
      positive = true;
    }
    else if (x < -crossingThreshold) {
      positive = false;
    }

    if (positive != previousPositive) {
      crossings++;
      previousPositive = positive;
    }
  }

  AudioInfo info;
  info.rms = sqrt(sumSq / SAMPLES_PER_WINDOW);

  float windowSeconds =
      (float)SAMPLES_PER_WINDOW / (float)SAMPLE_RATE;

  if (crossings > 1) {
    info.frequency = (crossings / 2.0) / windowSeconds;
  }
  else {
    info.frequency = 0.0;
  }

  return info;
}

bool sirenLike(AudioInfo left, AudioInfo right) {
  float strongestRms = max(left.rms, right.rms);
  float averageRms = (left.rms + right.rms) / 2.0;

  if (strongestRms < MIN_RMS) {
    return false;
  }

  bool leftFreqOK =
      left.frequency >= SIREN_MIN_HZ &&
      left.frequency <= SIREN_MAX_HZ;

  bool rightFreqOK =
      right.frequency >= SIREN_MIN_HZ &&
      right.frequency <= SIREN_MAX_HZ;

  if (averageRms < (MIN_RMS * 0.55)) {
    return false;
  }

  return leftFreqOK || rightFreqOK;
}

void updateDirection(float leftRms, float rightRms) {
  if (leftRms > rightRms * DIRECTION_MARGIN) {
    currentDirection = DIR_LEFT;
  }
  else if (rightRms > leftRms * DIRECTION_MARGIN) {
    currentDirection = DIR_RIGHT;
  }
  else {
    currentDirection = DIR_FRONT;
  }
}

void showNormalScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 5);
  display.println("SYSTEM");

  display.setCursor(10, 30);
  display.println("NORMAL");

  display.setTextSize(1);
  display.setCursor(15, 52);
  display.println("Monitoring sound");

  display.display();
}

void showEmergencyScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(25, 0);
  display.println("EMERGENCY");

  display.setTextSize(2);
  display.setCursor(15, 14);
  display.println("SIREN");

  display.setTextSize(1);

  if (currentDirection == DIR_LEFT) {
    display.setCursor(5, 38);
    display.println("< < VEHICLE");
    display.setCursor(5, 51);
    display.println("DIRECTION: LEFT");
  }
  else if (currentDirection == DIR_RIGHT) {
    display.setCursor(5, 38);
    display.println("VEHICLE > >");
    display.setCursor(5, 51);
    display.println("DIRECTION: RIGHT");
  }
  else {
    display.setCursor(22, 38);
    display.println("VEHICLE");
    display.setCursor(28, 51);
    display.println("IN FRONT");
  }

  display.display();
}

void updateLeds() {
  digitalWrite(RED_LED, sirenDetected ? HIGH : LOW);
  digitalWrite(GREEN_LED, sirenDetected ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  analogReadResolution(12);
  analogSetPinAttenuation(MIC_LEFT, ADC_11db);
  analogSetPinAttenuation(MIC_RIGHT, ADC_11db);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found at 0x3C");
    while (true) {
      digitalWrite(RED_LED, HIGH);
      delay(200);
      digitalWrite(RED_LED, LOW);
      delay(200);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(15, 10);
  display.println("SMART EMERGENCY");

  display.setCursor(27, 25);
  display.println("VEHICLE ALERT");

  display.setCursor(38, 45);
  display.println("ESP32 SYSTEM");

  display.display();

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  delay(2000);
  showNormalScreen();

  Serial.println();
  Serial.println("======================================");
  Serial.println(" Smart Emergency Vehicle Alert System");
  Serial.println("======================================");
  Serial.println("Left Mic  : GPIO34");
  Serial.println("Right Mic : GPIO35");
  Serial.println("OLED SDA  : GPIO21");
  Serial.println("OLED SCL  : GPIO22");
  Serial.println("OLED Addr : 0x3C");
  Serial.println();
  Serial.println("Watch RMS/frequency values.");
  Serial.println("Adjust MIN_RMS if false detection occurs.");
  Serial.println();
}

void loop() {
  AudioInfo left = analyzeMic(MIC_LEFT);
  AudioInfo right = analyzeMic(MIC_RIGHT);

  bool possibleSiren = sirenLike(left, right);

  if (possibleSiren) {
    goodCount++;
    badCount = 0;

    if (goodCount >= DETECT_CONFIRM_COUNT) {
      sirenDetected = true;
    }
  }
  else {
    badCount++;
    goodCount = 0;

    if (badCount >= RELEASE_COUNT) {
      sirenDetected = false;
    }
  }

  if (sirenDetected) {
    updateDirection(left.rms, right.rms);
  }

  updateLeds();

  Serial.print("L_RMS=");
  Serial.print(left.rms, 1);
  Serial.print(" L_F=");
  Serial.print(left.frequency, 0);
  Serial.print("Hz | R_RMS=");
  Serial.print(right.rms, 1);
  Serial.print(" R_F=");
  Serial.print(right.frequency, 0);
  Serial.print("Hz | ");

  if (sirenDetected) {
    Serial.print("SIREN=YES | DIR=");

    if (currentDirection == DIR_LEFT)
      Serial.println("LEFT");
    else if (currentDirection == DIR_RIGHT)
      Serial.println("RIGHT");
    else
      Serial.println("FRONT");
  }
  else {
    Serial.println("SIREN=NO");
  }

  if (sirenDetected)
    showEmergencyScreen();
  else
    showNormalScreen();
}
