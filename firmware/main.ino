SYSTEM_MODE(SEMI_AUTOMATIC);

#include "neopixel.h"

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT 88
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

int led = D7;

int pin = A0;
int readDelay = 5;

#define COLOR_LOOP_TIME 6000
int colorTracker = 170;
int lastColor = 0;

// data sets in .05 seconds
#define DATA_COUNT 10
int data[DATA_COUNT];
int dataIndex = 0;
int localAverage;

// average sets in 1.5 seconds
#define AVG_COUNT 30
int averages[AVG_COUNT];
int averageIndex = 0;

int maxHistory = readDelay * DATA_COUNT * AVG_COUNT * 2;

#define THRESHOLD 40
#define OFF 0
#define ON  1
int status = OFF;
int offValue = 4096;
int onValue = 0;
int diff;

long onStart;
long offStart;

long startTime;
bool gracePeriod;
int  graceTime = maxHistory;

void setup() {
  pinMode(led, OUTPUT);

  strip.begin();
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 10, 10, 10);
  }
  strip.show();

  Particle.connect();

  Particle.variable("diff", diff);
  Particle.subscribe("color", receiveColor);

  startTime = millis();
  gracePeriod = true;
}

void loop() {
  determineState();
  display();
}

void determineState() {
  checkGracePeriod();
  checkSensor();

  if (status == ON) {
    int timePassed = (millis() - onStart) % COLOR_LOOP_TIME;
    int colorAdjustment = map(timePassed, 0, COLOR_LOOP_TIME, 0, 255);
    colorTracker = ((lastColor + colorAdjustment) % 254) + 1;
  } else if (status == OFF) {
    lastColor = colorTracker;
  }
}

long lastPrint = millis();
void display() {
  if (gracePeriod) {
    return;
  }

  digitalWrite(led, status);

  uint32_t color = wheel(colorTracker);

  if (millis() > lastPrint + 25) {
    Serial.print(localAverage);
    Serial.print(",");
    Serial.print(offValue);
    Serial.print(",");
    Serial.print(status == ON ? 1400 : 1380);
    Serial.print(",");
    Serial.println(onValue);
  }
  // if (millis() > lastPrint + 250) {
  //   lastPrint = millis();
  //   if (status == ON) {
  //     Serial.println("offValue:     " + String(offValue));
  //     Serial.println("onvalue:      " + String(onValue) + " *");
  //   } else {
  //     Serial.println("offValue:     " + String(offValue) + " *");
  //     Serial.println("onValue:      " + String(onValue));
  //   }
  //   Serial.println("diff:         " + String(onValue - offValue));
  //   Serial.println("localAverage: " + String(localAverage));
  //   Serial.println("---------");
  // }

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void checkGracePeriod() {
  if (gracePeriod && (millis() - startTime > graceTime)) {
    // give the history trackers time to set up valid bounds
    gracePeriod = false;
  }
}

void checkSensor() {
  data[dataIndex++] = makeReading();
  if (dataIndex == DATA_COUNT) {
    dataIndex = 0;
    calculateLocalAverage();

    if (!gracePeriod) {
      if (status == OFF && localAverage > (offValue + THRESHOLD)) {
        status = ON;
        onStart = millis();
      } else if (status == ON && localAverage < (onValue - THRESHOLD)) {
        status = OFF;
        offStart = millis();
        Particle.publish("color", String(colorTracker));
      }
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
    if (offStart + maxHistory > millis()) {
      offValue = minimumAverage();
    } else {
      offValue = previousAverage();
    }
  }

  if (status == ON) {
    onValue = maximumAverage();
  }

  diff = onValue - offValue;
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

int previousAverage() {
  int sum = 0;
  for (int i = 0; i < AVG_COUNT; i++) {
    sum += averages[i];
  }

  return sum / AVG_COUNT;
}

void receiveColor(const char *event, const char *data) {
  int newColor = atoi(data);

  if (newColor != 0) {
    for (int i = 0; i < 100; i++) {
      int mapper = map(i, 0, 100, colorTracker, newColor);

      uint32_t color = wheel(mapper);
      for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
      }
      strip.show();
      delay(3);
    }

    colorTracker = newColor;
  }
}

uint32_t wheel(int position) {
  // Input a value 0 to 255 to get a color value.
  // The colors are a transition g - r - b.
  if(position < 85) {
   return strip.Color(position * 3, 255 - position * 3, 0);
 } else if(position < 170) {
   position -= 85;
   return strip.Color(255 - position * 3, 0, position * 3);
  } else {
   position -= 170;
   return strip.Color(0, position * 3, 255 - position * 3);
  }
}
