SYSTEM_MODE(SEMI_AUTOMATIC);

#include "dotstar.h"

#define PIXEL_COUNT 18
#define DATAPIN   A5
#define CLOCKPIN  A4
Adafruit_DotStar strip = Adafruit_DotStar(PIXEL_COUNT, DATAPIN, CLOCKPIN);

#define COLOR_LOOP_TIME 6000
int colorTracker = 170;
int lastColor = 0;

int sensorPin = D7;
int status = 0;

bool startupPing = false;
long selfPingTimer;

void setup() {
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
  checkSensor();

  if (status == ON) {
    int timePassed = (millis() - onStart) % COLOR_LOOP_TIME;
    int colorAdjustment = map(timePassed, 0, COLOR_LOOP_TIME, 0, 255);
    colorTracker = ((lastColor + colorAdjustment) % 254) + 1;
  } else if (status == OFF) {
    lastColor = colorTracker;
  }
}

void checkSensor() {
  status = digitalRead(sensorPin)
}

void display() {
  if (!startupPing) {
    Particle.publish("fl_ping");
    selfPingTimer = millis();
    startupPing = true;

    return;
  }

  uint32_t color = wheel(colorTracker);

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
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
