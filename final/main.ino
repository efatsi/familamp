SYSTEM_MODE(MANUAL);

int out = A0;
int led = A2;

void setup() {
  pinMode(out, INPUT);
  pinMode(led, INPUT);
}

void loop() {
  if (digitalRead(out)) {
    Serial.println("!!!");
  }

  delay(100);
}
