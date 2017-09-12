#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define ONE_WIRE_BUS 2 // DS18B20 on arduino pin2 corresponds to D4 on physical board

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;
ESP8266WebServer server(80);

#include "settings.h"

float displayTemp = 0.0f;
unsigned long displayRefreshTime = 0;
unsigned long displayLoopTimer = 0;

void runDisplay() {
  // Delay
  if ((millis() - displayRefreshTime) < 200) {
    return;
  }

  // Reset displayRefreshTime
  displayRefreshTime = millis();
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  
  display.print("uptime: ");
  display.println((float) millis() / 1000, 1);

  display.print("loop: ");
  display.println(displayLoopTimer);

  display.print("SSID: ");
  display.println(wifi_ssid);

  display.print("WiFi: ");
  display.println(WiFi.status());

  display.print("IP: ");
  display.println(WiFi.localIP());

  display.setCursor(60,48);
  display.setTextSize(2);
  display.print(displayTemp);
  display.setTextSize(1);
  display.setCursor(120,48);
  display.print("C");
  display.println();
  
  display.display();
}

unsigned long lastStableTime = 0;
bool checkStability() {
  if ((millis() - lastStableTime) > (10 * 1000)) {
    ESP.restart();
    return false;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  lastStableTime = millis();
  return true;
}

void setup() {
  // put your setup code here, to run once:
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();

  WiFi.begin(wifi_ssid, wifi_password);

  mdns.begin("esp8266", WiFi.localIP());

  server.on("/", [](){
    server.send(200, "text/html", "Hello");
  });

  server.begin();
}

void loop() {
  unsigned long loopTimer = millis();

  checkStability();
  
  server.handleClient();
  
  runDisplay();
  
  DS18B20.requestTemperatures(); 
  displayTemp = DS18B20.getTempCByIndex(0);

  displayLoopTimer = (millis() - loopTimer);
}
