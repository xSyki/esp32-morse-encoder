#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_CS     5
#define OLED_DC     27
#define OLED_RESET  14

Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  delay(2000);
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0)) {
    Serial.println(F("OLED failed to initialize"));
    for (;;);
  }

  Serial.println(F("OLED initialized!"));
  
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Hello, OLED!");
  
  display.display();
}

void loop() {

}
