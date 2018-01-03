SYSTEM_MODE(SEMI_AUTOMATIC);
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

#include "neopixel.h"

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

// average sets in 1 seconds
#define AVG_COUNT 20
int averages[AVG_COUNT];
int averageIndex = 0;

int maxHistory = readDelay * DATA_COUNT * AVG_COUNT;

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
int  graceTime = maxHistory * 2;
int  threshold;

bool startupPing = false;
long selfPingTimer;

void setup() {
  pinMode(led, OUTPUT);

  strip.begin();
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 10, 10, 10);
  }
  strip.show();

  Particle.connect();

  Particle.variable("diff", diff);
  Particle.function("setWifi", setWifi);

  Particle.subscribe("fl_ping", receivePing);
  Particle.subscribe("fl_color", receiveColor);

  EEPROM.get(0, threshold);

  startTime = millis();
  gracePeriod = true;
}

void loop() {
  // TODO: if on for over 30 seconds, bail out

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
  } else if (!startupPing) {
    Particle.publish("fl_ping");
    selfPingTimer = millis();
    startupPing = true;

    return;
  }

  digitalWrite(led, status);

  uint32_t color = wheel(colorTracker);

  // if (millis() > lastPrint + 25) {
  //   Serial.print(localAverage);
  //   Serial.print(",");
  //   Serial.print(offValue);
  //   Serial.print(",");
  //   Serial.print(status == ON ? 1400 : 1380);
  //   Serial.print(",");
  //   Serial.println(onValue);
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
      if (status == OFF && localAverage > (offValue + threshold)) {
        status = ON;
        onValue = localAverage;
        onStart = millis();
        Serial.println("Starting " + String(onStart));
      } else if (status == ON && localAverage < (onValue - threshold)) {
        status = OFF;
        offStart = millis();
        offValue = localAverage;
        Serial.println("Ending   " + String(offStart));
        Serial.println("Duration " + String((offStart - onStart) / 1000.0));
        Particle.publish("fl_color", String(colorTracker));
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
    if (offStart + (maxHistory * 1.5) < millis()) {
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

  if (colorTracker == newColor) {
    return;
  }

  if (newColor != 0) {
    for (int i = 0; i < 100; i++) {
      int mapper = map(i, 0, 100, colorTracker, newColor);

      uint32_t color = wheel(mapper);
      for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
      }
      strip.show();
      delay(8);
    }

    colorTracker = newColor;
  }
}

void receivePing(const char *event, const char *data) {
  if (selfPingTimer + 5000 < millis()) {
    Particle.publish("fl_color", String(colorTracker));
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

int setWifi(String command) {
  int seperatorIndex = command.indexOf(":");

  if (seperatorIndex > 0) {
    String ssid     = command.substring(0, seperatorIndex);
    String password = command.substring(seperatorIndex + 1);

    WiFi.setCredentials(ssid, password);
    return 1;
  } else {
    return 0;
  }
}
