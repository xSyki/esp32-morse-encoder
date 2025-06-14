#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define OLED_MOSI 23
#define OLED_CLK 18
#define OLED_CS 5
#define OLED_DC 27
#define OLED_RESET 14

#define BUTTON_MORSE 4
#define BUTTON_SEND 15

#define DOT_THRESHOLD_MS 300
#define SYMBOL_PAUSE_MS 500
#define WORD_PAUSE_MS 1200
#define BUTTON_DEBOUNCE_MS 30

const char *WIFI_SSID = "";
const char *WIFI_PASS = "";
const char *SERVER_URL = "";

void connectToWifi();
void updateDisplay();
void sendMorse();

Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);

String morseBuffer = "";
unsigned long pressStart = 0;
unsigned long lastRelease = 0;
bool isPressing = false;

void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(BUTTON_MORSE, INPUT_PULLUP);
  pinMode(BUTTON_SEND, INPUT_PULLUP);

  connectToWifi();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0))
  {
    Serial.println("OLED init failed");
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Morse ready");
  if (WiFi.status() == WL_CONNECTED)
  {
    display.println("WiFi connected");
    display.println(WiFi.localIP());
  }
  else
  {
    display.println("WiFi not connected");
  }
  display.display();
}

void connectToWifi()
{
  if (strlen(WIFI_SSID) == 0 || strlen(WIFI_PASS) == 0)
  {
    Serial.println("WiFi disabled (empty config)");
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries++ < 20)
  {
    delay(2000);
    Serial.print("Trying to connect");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("\nWiFi not connected – continuing offline");
    return;
  }

  Serial.println("\nWiFi connected");
}

void loop()
{
  unsigned long now = millis();
  bool morsePressed = digitalRead(BUTTON_MORSE) == LOW;
  bool sendPressed = digitalRead(BUTTON_SEND) == LOW;

  if (morsePressed && !isPressing)
  {
    pressStart = now;
    isPressing = true;
  }

  if (!morsePressed && isPressing)
  {
    unsigned long pressDuration = now - pressStart;
    isPressing = false;
    lastRelease = now;

    if (pressDuration < BUTTON_DEBOUNCE_MS)
      return;

    if (pressDuration < DOT_THRESHOLD_MS)
    {
      morseBuffer += ".";
    }
    else
    {
      morseBuffer += "-";
    }

    updateDisplay();
  }

  bool lastCharIsSpace = morseBuffer.length() > 0 && morseBuffer[morseBuffer.length() - 1] == ' ';

  if (!morsePressed && (now - lastRelease > WORD_PAUSE_MS) && morseBuffer.length() > 0 && !lastCharIsSpace)
  {
    morseBuffer += " ";
    lastRelease = now + 999999;
    updateDisplay();
  }

  if (sendPressed)
  {
    sendMorse();
    morseBuffer = "";
    updateDisplay();
    delay(400);
  }

  delay(10);
}

void updateDisplay()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Morse:");
  display.drawLine(0, 9, display.width(), 9, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.println(morseBuffer);

  display.display();
}

void sendMorse()
{
  morseBuffer.trim();

  if (morseBuffer.length() == 0)
  {
    Serial.println("No morse to send");
    return;
  }

  Serial.println("Sending morse:");
  Serial.println(morseBuffer);

  if (strlen(SERVER_URL) == 0)
  {
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    return;
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"morse\":\"" + morseBuffer + "\"}";
  int responseCode = http.POST(json);

  if (responseCode > 0)
  {
    Serial.print("Server response: ");
    Serial.println(responseCode);
  }
  else
  {
    Serial.println("Failed to send, skipping...");
  }

  http.end();
}
