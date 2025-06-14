#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

// OLED SPI pins
#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_CS     5
#define OLED_DC     27
#define OLED_RESET  14

// Button pins
#define BUTTON_MORSE  4
#define BUTTON_SEND   15

// Configuration
#define DOT_THRESHOLD_MS      300
#define SYMBOL_PAUSE_MS       500
#define WORD_PAUSE_MS         1200
#define BUTTON_DEBOUNCE_MS    30

// Optional Wi-Fi (set to "" to disable)
const char* WIFI_SSID = ""; // e.g. "MyNetwork"
const char* WIFI_PASS = ""; // e.g. "password123"
const char* SERVER_URL = ""; // e.g. "http://192.168.0.123:5000/morse"

Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);

// Morse state
String morseBuffer = "";
unsigned long pressStart = 0;
unsigned long lastRelease = 0;
bool isPressing = false;

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(BUTTON_MORSE, INPUT_PULLUP);
  pinMode(BUTTON_SEND, INPUT_PULLUP);

  // Wi-Fi (optional)
  if (strlen(WIFI_SSID) > 0 && strlen(WIFI_PASS) > 0) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries++ < 20) {
      delay(300);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected");
    } else {
      Serial.println("\nWiFi not connected – continuing offline");
    }
  } else {
    Serial.println("WiFi disabled (empty config)");
  }

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0)) {
    Serial.println("OLED init failed");
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Morse ready");
  display.display();
}

void loop() {
  unsigned long now = millis();
  bool morsePressed = digitalRead(BUTTON_MORSE) == LOW;
  bool sendPressed = digitalRead(BUTTON_SEND) == LOW;

  // --- Morse input logic ---
  if (morsePressed && !isPressing) {
    pressStart = now;
    isPressing = true;
  }

  if (!morsePressed && isPressing) {
    unsigned long pressDuration = now - pressStart;
    isPressing = false;
    lastRelease = now;

    if (pressDuration < BUTTON_DEBOUNCE_MS) return;

    if (pressDuration < DOT_THRESHOLD_MS) {
      morseBuffer += ".";
    } else {
      morseBuffer += "-";
    }

    updateDisplay();
  }

  // Detect pause (symbol or word)
  if (!morsePressed && (now - lastRelease > WORD_PAUSE_MS) && morseBuffer.length() > 0) {
    morseBuffer += " ";
    lastRelease = now + 999999; // prevent repeat
    updateDisplay();
  }

  // --- SEND button logic ---
  if (sendPressed) {
    sendMorse();
    morseBuffer = "";
    updateDisplay();
    delay(400); // debounce and UX pause
  }

  delay(10);
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Morse:");
  display.setCursor(0, 10);
  display.println(morseBuffer);
  display.display();
}

void sendMorse() {
  Serial.println("Sending morse:");
  Serial.println(morseBuffer);

  if (strlen(SERVER_URL) == 0) {
    Serial.println("No server URL defined – skipping send");
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No WiFi – skipping send");
    return;
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"morse\":\"" + morseBuffer + "\"}";
  int responseCode = http.POST(json);

  if (responseCode > 0) {
    Serial.print("Server response: ");
    Serial.println(responseCode);
  } else {
    Serial.println("Failed to send, skipping...");
  }

  http.end();
}
