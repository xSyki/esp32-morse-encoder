void setup() {
  delay(2000);
  Serial.begin(115200);

  Serial.println("Hello from ESP32!");

  pinMode(2, OUTPUT);

  Serial.println("Pin setup");
}

void loop() {
  Serial.println("ESP32 is running...");

  digitalWrite(2, HIGH);

  Serial.println("Diod HIGH");

  delay(500);

  digitalWrite(2, LOW);

  Serial.println("Diod LOW");

  delay(500);
}