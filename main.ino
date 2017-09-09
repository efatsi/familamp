SYSTEM_MODE(MANUAL);

int pin = A0;

int readDelay = 10;

#define DATA_COUNT 10
#define AVG_COUNT 20

int data[DATA_COUNT];
int dataIndex = 0;
int localAverage;

int averages[AVG_COUNT];
int averageIndex = 0;
int globalAverage;

void setup() {
  for (int i = 0; i < DATA_COUNT; i++) {
    data[i] = makeReading();
    delay(readDelay);
  }
}

void loop() {
  data[dataIndex++] = makeReading();;
  if (dataIndex == DATA_COUNT) {
    dataIndex = 0;
    calculateLocalAverage();
  }

  delay(readDelay);
}

int makeReading() {
  // write HIGH
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);

  // read on same pin
  pinMode(pin, INPUT);
  analogRead(pin);
}

void calculateLocalAverage() {
  int sum = 0;
  for (int i = 0; i < DATA_COUNT; i++) {
    sum += data[i];
  }
  localAverage = sum / DATA_COUNT;
}

// if OFF
//   take last 5 localAverages, pick the lowest, that's the "off value"
//   if new localAverage is >40, switch to ON

// if ON
//   take last 5 localAverages, pick the highest, that's the "on value"
//   if new localAverage is <40, switch to OFF
