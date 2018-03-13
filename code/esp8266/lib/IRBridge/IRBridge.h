/* IRBridge.h

Purpose
=======

This is the main IRBridge library encapsulating IR related functionality.

For sending IR events, the library [IRRemoteESP8266](https://github.com/markszabo/IRremoteESP8266) is used.
For Alexa-functionality (emulating Belkin WeMo), the [fauxmoesp library](https://bitbucket.org/xoseperez/fauxmoesp) is used

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

#ifndef IRBridge_h
#define IRBridge_h

#include <WebController.h>
#include "ESPAsyncTCP.h"
#include <ESPAsyncWebServer.h>

#include <FS.h>
#include <Logger.h>

#include <fauxmoESP.h>

#define ARDUINOJSON_USE_LONG_LONG 1
#include <AsyncJson.h>
#include <ArduinoJson.h>

#include "FilesToJSON.hpp"
#include "IRQueue.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

class IRBridge
{
public:

  IRBridge();

  // should be called in Arduino setup function
  void setup();

  // should be called in Arduino loop function
  void loop();

protected:


private:

  WebController controller;

  // general IR related stuff
  IRQueue irqueue;
  IRrecv irrecv;
  void handleSendIR(AsyncWebServerRequest * request);

  // functions and variables for bookmarks
  String bookmarksprefix;
  bool sendIR(JsonObject& root); // sends an IR code for a bookmark given as a JsonObject
  bool sendIRProtocol(decode_type_t prot, uint64_t value, int bits, int repeat);
  bool validateCommandJson(JsonObject& root);
  bool bookmarkExists(String bookmarkName);
  bool bookmarkLengthOk(String bookmarkName);
  String bookmarkUsedInDevice(String bookmarkName);
  void propagateBookmarkRename(String oldBookmarkName, String newBookmarkName);
  bool validateBookmarkJson(JsonObject& root);
  void whitelistBookmarkJson(JsonObject& root, std::vector<String> whitelist);

  void handleGetBookmarks(AsyncWebServerRequest * request);
  void handleAddBookmark(AsyncWebServerRequest * request);
  void handleDeleteBookmark(AsyncWebServerRequest * request, String bookmarkname);
  void handleEditBookmark(AsyncWebServerRequest * request, String bookmarkname);

  // functions and variables for devices
  const char* devicesfilename;
  bool sendIR(String devicename, bool status); // sends an IR code for a device
  bool validateDeviceJson(JsonObject& root);

  void handleGetDevices(AsyncWebServerRequest * request);
  void handleAddDevice(AsyncWebServerRequest * request);
  void handleDeleteDevice(AsyncWebServerRequest * request, String devicename);
  void handleEditDevice(AsyncWebServerRequest * request, String devicename);

  // variables and functions for Alexa support
  void setupAlexa();
  fauxmoESP alexa;
  bool alexaEnabled;


};

#endif
