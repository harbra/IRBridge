/* WebController.h

Purpose
=======

The WebController bundles basic functionality that is related to the ESP's web connectivity

It provides the following functionality:
* Automatically connect to a configured wifi network
* If that fails, create an access point that can be used to connect the device to an existing wifi network
* Provide basic information on the ESP's perfomance characteristics (e.g. free heap, etc.)


License
=======

MIT License

Copyright (c) 2018 Harald Braun

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef WebController_h
#define WebController_h

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>

#include <ESPAsyncWebServer.h>
#include <AsyncEventSource.h>

extern "C" {
#include "libb64/cdecode.h"
}

#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>
#include <Logger.h>

#define ARDUINOJSON_USE_LONG_LONG 1
#include <AsyncJson.h>
#include <ArduinoJson.h>

#include "FilesToJSON.hpp"
#include "IRQueue.h"

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

class WebHandlerContainer
{
public:
  WebHandlerContainer(const char* uri, WebRequestMethodComposite method, std::function<void(AsyncWebServerRequest *request, String parameters)> onRequest) : myUri(uri), myMethod(method), myOnRequest(onRequest) {
    if(!myUri.endsWith("/")) {
      myUri+="/";
    }
  };

  String myUri;
  WebRequestMethodComposite myMethod;
  std::function<void(AsyncWebServerRequest *request, String parameters)> myOnRequest;
};

class WebController
{
public:

  WebController();

  // should be called in Arduino setup function
  void setup();

  // should be called in Arduino loop function
  void loop();

  // can be used to dispatch a Server-Sent Event to connected clients
  void sendEvent(String eventname, String eventcontent);

  void addSettingsIncludePath(String path);
  void resetSettingsIncludePaths();

  void addSettingsBlacklistFilename(String filename);
  void resetSettingsBlacklist();

  void addServerHandler(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, bool acceptFileUpload, bool acceptBodyUpload);
  void addServerHandler(const char* uri, WebRequestMethodComposite method, std::function<void(AsyncWebServerRequest *request, String parameters)> onRequest);

protected:


private:

  void setupServer();
  bool connectWifi();
  void createAP();

  String getContentType(String filename);
  bool handleFileRead(AsyncWebServerRequest *request, String path);
  bool handleGetSettings(AsyncWebServerRequest *request);
  bool handleSetSettings(AsyncWebServerRequest *request);
  bool handleScanWifi(AsyncWebServerRequest *request);
  void handleRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
  void handleFileUploadDummy(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
  void handleGetInfo(AsyncWebServerRequest * request);
  void handleGetWifiStatus(AsyncWebServerRequest * request);
  void handleReboot(AsyncWebServerRequest * request);
  void handleDownloadSettings(AsyncWebServerRequest * request);
  void handleRestoreSettings(AsyncWebServerRequest * request);

  void handleTest(AsyncWebServerRequest *request);

  std::pair<int, std::unique_ptr<char[]>> decode64(const char* data, size_t length);

  String getConnectionStatus();
  void resetWifiConfig();
  void restoreSettings();

  // variables for server
  const char* settingsfilename;
  String pathprefix;
  AsyncWebServer server;
  AsyncEventSource events;
  bool resetWifi;
  String restoreSettingsFile;
  bool reboot;
  std::vector<String> settings_includepaths;
  std::vector<String> settings_filesblacklist;
  std::vector<WebHandlerContainer> additionalWebHandlers;

  // timeouts
  unsigned long previousMillisConnectionCheck;
  unsigned long timeoutConnectionCheck;

  // variables for DNS server
  DNSServer dnsserver;

  // parameters for an access point created by the ESP
  char ap_ssid[32];
  char ap_pwd[64];

  // parameters for the network the ESP should connect to
  char sta_ssid[32];
  char sta_pwd[64];
  bool sta_dhcp;
  char sta_ip[15];
  char sta_gw[15];
  char sta_sn[15];

  bool loadSettings(String filename);
  bool saveSettings(String filename);

};

#endif
