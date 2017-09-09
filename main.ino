SYSTEM_MODE(MANUAL);

int led = D7;

int pin = A0;
int readDelay = 5;

#define DATA_COUNT 10
int data[DATA_COUNT];
int dataIndex = 0;
int localAverage;

#define AVG_COUNT 25
int averages[AVG_COUNT];
int averageIndex = 0;

#define THRESHOLD 25
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
}

long lastPrint = millis();
void loop() {
  if (gracePeriod && (millis() - startTime > graceTime)) {
    // give the history trackers time to set up valid bounds
    gracePeriod = false;
  }

  if (millis() > lastPrint + 1000) {
    lastPrint = millis();
    Serial.println("offValue:     " + String(offValue));
    Serial.println("onValue:      " + String(onValue));
    Serial.println("localAverage: " + String(localAverage));
    Serial.println("---------");
  }

  digitalWrite(led, status);

  data[dataIndex++] = makeReading();
  if (dataIndex == DATA_COUNT) {
    dataIndex = 0;
    calculateLocalAverage();

    if (status == OFF && localAverage > (offValue + THRESHOLD) && !gracePeriod) {
      status = ON;
      onValue = localAverage;
    } else if (status == ON && localAverage < (onValue - THRESHOLD) && !gracePeriod) {
      status = OFF;
      offValue = localAverage;
    } else {
      recordLocalAverage();
    }
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
