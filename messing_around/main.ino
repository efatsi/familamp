SYSTEM_MODE(MANUAL);

void setup() {
  System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, "Photon");
  pinMode(D7, OUTPUT);
}

void loop() {

  digitalWrite(D7, HIGH);
  delay(1000);
  digitalWrite(D7, LOW);
  delay(1000);
}


FiOS-HWXYN
facts2789sod70spry
