#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

// OLED pins
#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_CS     5
#define OLED_DC     27
#define OLED_RESET  14

// Button pins
#define BUTTON_LEFT   4
#define BUTTON_RIGHT  15

Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);

// Timers
unsigned long leftStartTime = 0;
unsigned long rightStartTime = 0;

unsigned long leftHeldTime = 0;
unsigned long rightHeldTime = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0)) {
    Serial.println("OLED init failed");
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Hold buttons...");
  display.display();
}

void loop() {
  bool leftPressed = digitalRead(BUTTON_LEFT) == LOW;
  bool rightPressed = digitalRead(BUTTON_RIGHT) == LOW;
  unsigned long now = millis();

  // LEFT
  if (leftPressed && leftStartTime == 0) {
    leftStartTime = now;
  } else if (!leftPressed && leftStartTime != 0) {
    leftHeldTime = now - leftStartTime;
    leftStartTime = 0;
  }

  // RIGHT
  if (rightPressed && rightStartTime == 0) {
    rightStartTime = now;
  } else if (!rightPressed && rightStartTime != 0) {
    rightHeldTime = now - rightStartTime;
    rightStartTime = 0;
  }

  // Realtime update
  unsigned long leftDisplay = leftPressed ? now - leftStartTime : leftHeldTime;
  unsigned long rightDisplay = rightPressed ? now - rightStartTime : rightHeldTime;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("LEFT: ");
  display.print(leftDisplay);
  display.println(" ms");

  display.setCursor(0, 10);
  display.print("RIGHT: ");
  display.print(rightDisplay);
  display.println(" ms");

  display.display();
  delay(50);
}
