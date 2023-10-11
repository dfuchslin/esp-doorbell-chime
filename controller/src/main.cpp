#define XSTR(x) #x
#define STR(x) XSTR(x)

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWiFiManager.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncUDP.h>

#pragma message "Setting hostname to " STR(HOSTNAME)
const char *hostname = STR(HOSTNAME);

const uint8_t DOORBELL_PIN = D6;
const uint16_t UDP_PORT = 4711;

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
AsyncUDP udp;

unsigned long doorbellLastTriggered = 0;
bool doorbellTriggered = false;

void health(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "ok");
}

void status(AsyncWebServerRequest *request)
{
  String status = "{}";
  request->send(200, "application/json", status);
}

void metrics(AsyncWebServerRequest *request)
{
  String message = F("# HELP esp_up Is this host up\n");
  message += F("# HELP esp_up gauge\n");
  message += F("esp_up 1\n");

  request->send(200, "text/plain", message);
}

void reset(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "All network settings reset. Please reboot and reconfigure.");
  wifiManager.resetSettings();
}

void reboot(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "Rebooting");
  ESP.reset();
}

void ring(AsyncWebServerRequest *request)
{
  doorbellTriggered = true;
  request->send(200, "text/plain", "ring");
}

void routing()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", hostname); });
  server.on("/health", HTTP_GET, health);
  server.on("/metrics", HTTP_GET, metrics);
  server.on("/status", HTTP_GET, status);
  server.on("/reset", HTTP_DELETE, reset);
  server.on("/reboot", HTTP_POST, reboot);
  server.on("/ring", HTTP_GET, ring);
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "Not found"); });
}

void IRAM_ATTR handleDoorbellInterrupt()
{
  doorbellTriggered = true;
}

void handleDoorbellTrigger()
{
  if (doorbellTriggered)
  {
    doorbellTriggered = false;
    if (doorbellLastTriggered < (millis() - 500))
    {
      doorbellLastTriggered = millis();
      Serial.println("Doorbell triggered");
      // Send a few packets since some have been missed by the chimes
      for (int i = 0; i < 5; i++)
      {
        udp.broadcastTo("ring", UDP_PORT);
      }
    }
  }
}

void handleWiFiDisconnect(WiFiEvent_t event){
  Serial.print("Wifi disconnected, reconnecting...");
  WiFi.setAutoReconnect(true);
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Starting up");

  Serial.print("Configuring wifi for");
  Serial.println(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname);
  WiFi.persistent(true); // https://github.com/alanswx/ESPAsyncWiFiManager/issues/83
  WiFi.onEvent(handleWiFiDisconnect, WIFI_EVENT_STAMODE_DISCONNECTED);
  wifiManager.autoConnect();

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPort(8266);
  ArduinoOTA.begin();

  MDNS.begin(hostname);
  MDNS.addService("http", "tcp", 80);

  routing();
  server.begin();

  pinMode(DOORBELL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(DOORBELL_PIN), handleDoorbellInterrupt, RISING);

  Serial.println("Ready");
}

void loop()
{
  MDNS.update();
  ArduinoOTA.handle();
  handleDoorbellTrigger();
}
