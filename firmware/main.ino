SYSTEM_MODE(MANUAL);

#include "neopixel.h"

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT 88
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

int led = D7;

int pin = A0;
int readDelay = 5;

#define DATA_COUNT 10
int data[DATA_COUNT];
int dataIndex = 0;
int localAverage;

#define AVG_COUNT 20 // one second of data (1000 / (readDelay * DATA_COUNT))
int averages[AVG_COUNT];
int averageIndex = 0;

#define THRESHOLD 30
#define OFF 0
#define ON  1
int status = OFF;
int offValue = 4096;
int onValue = 0;

long startTime;
bool gracePeriod;
int  graceTime = readDelay * DATA_COUNT * AVG_COUNT * 2;

void setup() {
  pinMode(led, OUTPUT);

  startTime = millis();
  gracePeriod = true;

  strip.begin();
  strip.show();
}

void loop() {
  determineState();
  display();
}

#define COLOR_LOOP_TIME 6000
int colorTracker = 85;
int lastColor = 0;
long onStart;
void determineState() {
  checkSensor();

  if (status == ON) {
    int timePassed = (millis() - onStart) % COLOR_LOOP_TIME;
    int colorAdjustment = map(timePassed, 0, COLOR_LOOP_TIME, 0, 255);
    colorTracker = (lastColor + colorAdjustment) % 255;
  } else if (status == OFF) {
    lastColor = colorTracker;
  }
}

long lastPrint = millis();
void display() {
  digitalWrite(led, status);

  uint32_t color = Wheel(colorTracker);

  if (millis() > lastPrint + 1000) {
    lastPrint = millis();
    Serial.println("offValue:     " + String(offValue));
    Serial.println("onValue:      " + String(onValue));
    Serial.println("localAverage: " + String(localAverage));
    Serial.println("colorTracker: " + String(color));
    Serial.println("color:        " + String(colorTracker));
    Serial.println("---------");
  }


  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void checkSensor() {
  if (gracePeriod && (millis() - startTime > graceTime)) {
    // give the history trackers time to set up valid bounds
    gracePeriod = false;
  }

  data[dataIndex++] = makeReading();
  if (dataIndex == DATA_COUNT) {
    dataIndex = 0;
    calculateLocalAverage();

    if (status == OFF && localAverage > (offValue + THRESHOLD) && !gracePeriod) {
      status = ON;
      onStart = millis();
    } else if (status == ON && localAverage < (onValue - THRESHOLD) && !gracePeriod) {
      status = OFF;
    }

    recordLocalAverage();
  }
}

int makeReading() {
  delay(readDelay);

  // write HIGH
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);

  // read on same pin
  pinMode(pin, INPUT);
  return analogRead(pin);
}

void calculateLocalAverage() {
  int sum = 0;
  for (int i = 0; i < DATA_COUNT; i++) {
    sum += data[i];
  }
  localAverage = sum / DATA_COUNT;
}

void recordLocalAverage() {
  averages[averageIndex++] = localAverage;
  averageIndex = averageIndex % AVG_COUNT;

  if (status == OFF) {
    offValue = minimumAverage();
  }

  if (status == ON) {
    onValue = maximumAverage();
  }
}

int minimumAverage() {
  int toReturn = 4096;
  for (int i = 0; i < AVG_COUNT; i++) {
    if (averages[i] != 0 && averages[i] < toReturn) {
      toReturn = averages[i];
    }
  }

  return toReturn;
}

int maximumAverage() {
  int toReturn = 0;
  for (int i = 0; i < AVG_COUNT; i++) {
    if (averages[i] > toReturn) {
      toReturn = averages[i];
    }
  }

  return toReturn;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
