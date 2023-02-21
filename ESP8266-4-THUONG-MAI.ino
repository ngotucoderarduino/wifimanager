
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
OneButton button1(14, true);  //D5
OneButton button2(12, true);  //D6
OneButton button3(13, true);  //D7
OneButton button4(3, true);   //RX
SimpleTimer timer;
int RL1 = 16; //D0
int RL2 = 5;  //D1
int RL3 = 4;  //D2
int RL4 = 0;  //D3
int LED = 2;  //D4
char blynk_token[100] = "";
bool shouldSaveConfig = false;
boolean checkData = false;
unsigned long times = millis();
unsigned long previousMillis = 0;
WidgetLED led_connect(V0);
BLYNK_WRITE(V1)
{
  int p = param.asInt();
  digitalWrite(RL1, !p);
  digitalWrite(LED, !p);
}
BLYNK_WRITE(V2)
{
  int p = param.asInt();
  digitalWrite(RL2, !p);
}
BLYNK_WRITE(V3)
{
  int p = param.asInt();
  digitalWrite(RL3, !p);
}
BLYNK_WRITE(V4)
{
  int p = param.asInt();
  digitalWrite(RL4, !p);
}
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
}

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
  pinMode(RL1, OUTPUT);
  pinMode(RL2, OUTPUT);
  pinMode(RL3, OUTPUT);
  pinMode(RL4, OUTPUT);
  digitalWrite(RL1, HIGH);
  digitalWrite(RL2, HIGH);
  digitalWrite(RL3, HIGH);
  digitalWrite(RL4, HIGH);
  pinMode(14, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  button1.attachClick(nhan_don1);
  button1.attachLongPressStart(nhan_giu1);
  button2.attachClick(nhan_don2);
  button3.attachClick(nhan_don3);
  button4.attachClick(nhan_don4);
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
  button1.tick();
  button2.tick();
  button3.tick();
  button4.tick();
}

void nhan_don1()
{
  digitalWrite(RL1, !digitalRead(RL1));
  digitalWrite(LED, digitalRead(RL1));
  Blynk.virtualWrite(V1, !digitalRead(RL1));
  delay(10);
}
void nhan_don2()
{
  digitalWrite(RL2, !digitalRead(RL2));
  Blynk.virtualWrite(V2, !digitalRead(RL2));
  delay(10);
}
void nhan_don3()
{
  digitalWrite(RL3, !digitalRead(RL3));
  Blynk.virtualWrite(V3, !digitalRead(RL3));
  delay(10);
}
void nhan_don4()
{
  digitalWrite(RL4, !digitalRead(RL4));
  Blynk.virtualWrite(V4, !digitalRead(RL4));
  delay(10);
}

void nhan_giu1()
{
  wifiManager.resetSettings();
  ESP.reset();
  delay(1000);
}
