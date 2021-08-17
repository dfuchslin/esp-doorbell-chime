#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWiFiManager.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include "AsyncUDP.h"

#include <Preferences.h>

#include <DFRobot_PLAY.h>
#include <HardwareSerial.h>

#define DFPLAYER_TX 14
#define DFPLAYER_RX 4
#define PREFS_VOLUME "volume"
#define PREFS_CHIME "chime"

const char *hostname = "doorbell-chime-hallway";
const uint16_t UDP_PORT = 4711;

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
AsyncUDP udp;

HardwareSerial DF1201SSerial(1);
DFRobot_PLAY DF1201S;
Preferences preferences;

unsigned long chimeLastTriggered = 0;
bool chimeTriggered = false;

void health(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "ok");
}

void status(AsyncWebServerRequest *request)
{
  String status = "{";
  char vol[16];
  snprintf(vol, sizeof(vol), "\"volume\":%u", DF1201S.getVol());
  status += String(vol);

  status += ",\"chimes\":{";

  char active[16];
  snprintf(active, sizeof(active), "\"active\":%u", preferences.getShort(PREFS_CHIME, 1));
  status += String(active) + ",";

  char total[16];
  snprintf(total, sizeof(total), "\"total\":%u", DF1201S.getTotalFile());
  status += String(total);
  status += "}";

  status += "}";

  request->send(200, "application/json", status);
}

void metrics(AsyncWebServerRequest *request)
{
  String message = F("# HELP esp8266_up Is this host up\n");
  message += F("# HELP esp8266_up gauge\n");
  message += F("esp8266_up 1\n");

  request->send(200, "text/plain", message);
}

void playActiveChime(AsyncWebServerRequest *request)
{
  DF1201S.setPlayMode(DF1201S.SINGLE);
  DF1201S.playFileNum(preferences.getShort(PREFS_CHIME, 1));
  status(request);
}

void playSpecificChime(AsyncWebServerRequest *request)
{
  int chime = request->pathArg(0).toInt();
  if (chime < 1 || chime > DF1201S.getTotalFile())
  {
    request->send(400, "text/plain", "Invalid chime '" + request->pathArg(0) + "'");
    return;
  }
  DF1201S.setPlayMode(DF1201S.SINGLE);
  DF1201S.playFileNum(chime);
  status(request);
}

void setActiveChime(AsyncWebServerRequest *request)
{
  int chime = request->pathArg(0).toInt();
  if (chime < 1 || chime > DF1201S.getTotalFile())
  {
    request->send(400, F("text/plain"), "Invalid chime '" + request->pathArg(0) + "'");
    return;
  }
  preferences.putShort(PREFS_CHIME, chime);
  status(request);
}

void setVolume(AsyncWebServerRequest *request)
{
  int volume = request->pathArg(0).toInt();
  if (volume < 0 || volume > 30)
  {
    request->send(400, "text/plain", "Invalid volume '" + request->pathArg(0) + "'");
    return;
  }
  preferences.putUChar(PREFS_VOLUME, volume);
  DF1201S.setVol(volume);
  status(request);
}

void reset(AsyncWebServerRequest *request)
{
  wifiManager.resetSettings();
  preferences.clear();
  request->send(200, "text/plain", "All network settings reset. Please reboot and reconfigure.");
}

void reboot(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "Rebooting");
  ESP.restart();
}

void routing()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, F("text/plain"), hostname); });
  server.on("/health", HTTP_GET, health);
  server.on("/metrics", HTTP_GET, metrics);
  server.on("/status", HTTP_GET, status);
  server.on("/reset", HTTP_DELETE, reset);
  server.on("/reboot", HTTP_POST, reboot);
  server.on("^\\/chime\\/([0-9]+)$", HTTP_GET, playSpecificChime);
  server.on("^\\/chime\\/([0-9]+)$", HTTP_POST, setActiveChime);
  server.on("/chime", HTTP_GET, playActiveChime);
  server.on("^\\/volume\\/([0-9]+)$", HTTP_POST, setVolume);
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "Not found"); });
}

void setupDFPLayer()
{
  preferences.begin("chime", false);

  DF1201SSerial.begin(115200, SERIAL_8N1, DFPLAYER_TX, DFPLAYER_RX);
  while (!DF1201S.begin(DF1201SSerial))
  {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
    // TODO show this error in status
  }
  DF1201S.setVol(10);
  //DF1201S.setPrompt(false);
  DF1201S.setLED(false);
  DF1201S.switchFunction(DF1201S.MUSIC);
  DF1201S.setPlayMode(DF1201S.SINGLE);

  DF1201S.setVol(preferences.getUChar(PREFS_VOLUME, 10));
}

void setupUDPServer()
{
  // https://github.com/me-no-dev/ESPAsyncUDP/blob/master/examples/AsyncUDPServer/AsyncUDPServer.ino
  if (udp.listen(UDP_PORT))
  {
    udp.onPacket([](AsyncUDPPacket packet)
                 { chimeTriggered = true; });
  }
}

void handleChimeTrigger()
{
  if (chimeTriggered)
  {
    chimeTriggered = false;
    if (chimeLastTriggered < (millis() - 250))
    {
      chimeLastTriggered = millis();
      Serial.println("Chime triggered");
      DF1201S.setPlayMode(DF1201S.SINGLE);
      DF1201S.playFileNum(preferences.getShort(PREFS_CHIME, 1));
    }
  }
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Starting up");

  WiFi.mode(WIFI_STA);
  WiFi.persistent(true); // https://github.com/alanswx/ESPAsyncWiFiManager/issues/83
  wifiManager.autoConnect();

  ArduinoOTA.setPort(8266);
  ArduinoOTA.begin();

  MDNS.begin(hostname);
  MDNS.addService("http", "tcp", 80);

  routing();
  server.begin();

  setupUDPServer();
  setupDFPLayer();

  Serial.println("Ready");
}

void loop(void)
{
  ArduinoOTA.handle();
  handleChimeTrigger();
}
