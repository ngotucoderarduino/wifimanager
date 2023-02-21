
#define BLYNK_PRINT Serial
#include <FS.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <OneButton.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
WiFiManager wifiManager;
#include <Ticker.h>
Ticker ticker;
#include <ArduinoJson.h>
SimpleTimer timer;
int LED = 2;  //D4
char blynk_token[100] = "";
bool shouldSaveConfig = false;
boolean checkData = false;
unsigned long times = millis();
unsigned long previousMillis = 0;
WidgetLED led_connect(V0);
void tick();
void configModeCallback (WiFiManager *myWiFiManager);
void saveConfigCallback ();
void saveConfigCallback ()
{
  shouldSaveConfig = true;
}
void configModeCallback (WiFiManager *myWiFiManager) {
  ticker.attach(0.2, tick);
}
void tick() {
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}
void setup()
{
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(LED, OUTPUT);

  ticker.attach(0.6, tick);
  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json"))
    {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success())
        {
          strcpy(blynk_token, json["blynk_token"]);
        }
        configFile.close();
      }
    }
  }
  WiFiManagerParameter custom_blynk_token("blynk", "Blynk Token", blynk_token, 100);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_blynk_token);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalTimeout (60);
  if (!wifiManager.autoConnect("THIẾT BỊ THÔNG MINH", "66668888"))
  {
    ESP.reset();
    delay(2000);
  }
  ticker.detach();
  strcpy(blynk_token, custom_blynk_token.getValue());
  if (shouldSaveConfig)
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;
    File configFile = SPIFFS.open("/config.json", "w");
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
  Blynk.config(blynk_token, "blynk.cloud", 80);
 // Blynk.config(blynk_token, "blynk.en-26.com", 9600);
  delay(1000);
}

void loop()
{
  unsigned long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= 30000))
  {
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  Blynk.run();
  timer.run();
  if (millis() - times > 500) {
    Blynk.virtualWrite(V0, millis() / 1000);
    if (led_connect.getValue()) {
      led_connect.off();
    }
    else {
      led_connect.on();
    }
    times = millis();
  }

}

