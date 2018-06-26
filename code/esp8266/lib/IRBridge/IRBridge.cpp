/*
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

#include "IRBridge.h"

IRBridge::IRBridge() : alexaEnabled(false), devicesfilename("/config/devices.json"), bookmarksprefix("/bookmarks/"), irqueue(12), irrecv(13)
{
}

void IRBridge::setup()
{
    myLogger.addToLog("Initializing IRBridge...");
    SPIFFS.begin();
    SPIFFS.remove("/temp/temp");

    controller.setup();

    controller.addSettingsIncludePath("/bookmarks/");
    controller.addSettingsIncludePath("/config/");
    controller.addSettingsIncludePath("/temp/");

    controller.addServerHandler("/api/bookmarks",HTTP_GET,std::bind(&IRBridge::handleGetBookmarks, this, std::placeholders::_1),false,false);
    controller.addServerHandler("/api/bookmarks",HTTP_POST,std::bind(&IRBridge::handleAddBookmark, this, std::placeholders::_1),false,true);
    controller.addServerHandler("/api/bookmarks",HTTP_DELETE,std::bind(&IRBridge::handleDeleteBookmark, this, std::placeholders::_1, std::placeholders::_2));
    controller.addServerHandler("/api/bookmarks",HTTP_PUT,std::bind(&IRBridge::handleEditBookmark, this, std::placeholders::_1, std::placeholders::_2));

    controller.addServerHandler("/api/devices",HTTP_GET,std::bind(&IRBridge::handleGetDevices, this, std::placeholders::_1),false,false);
    controller.addServerHandler("/api/devices",HTTP_POST,std::bind(&IRBridge::handleAddDevice, this, std::placeholders::_1),false,true);
    controller.addServerHandler("/api/devices",HTTP_DELETE,std::bind(&IRBridge::handleDeleteDevice, this, std::placeholders::_1, std::placeholders::_2));
    controller.addServerHandler("/api/devices",HTTP_PUT,std::bind(&IRBridge::handleEditDevice, this, std::placeholders::_1, std::placeholders::_2));

    controller.addServerHandler("/api/sendir",HTTP_GET,std::bind(&IRBridge::handleSendIR, this, std::placeholders::_1),false,false);

    irrecv.enableIRIn();

    setupAlexa();
}

void IRBridge::loop()
{
    controller.loop();

    if(alexaEnabled) {
      // TODO: memory leak in fauxmoesp & crash when calling .handle on it when no device has been added.
      alexa.handle();
    }

    // check if new IR signals have been received by the IR receiver led
    decode_results results;

    if (irrecv.decode(&results)) {
      // create a JSON object out of the decoded data and send the data via server-sent events
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();

      root["type"]=(int16_t)results.decode_type;
      root["type_String"]=typeToString(results.decode_type, results.repeat);
      root["value"]=results.value;//uint64ToString(results.value,16);
      root["address"]=results.address;//uint64ToString(results.address,16);
      root["command"]=results.command;//uint64ToString(results.command,16);
      root["bits"]=results.bits;
      root["overflow"]=results.overflow;
      root["repeat"]=results.repeat;

      String output;
      root.printTo(output);
      controller.sendEvent("IRreceived",output);
      irrecv.resume();  // Receive the next value
    }

    irqueue.process();
}

void IRBridge::setupAlexa()
{
  File devicesFile=SPIFFS.open(devicesfilename,"r");

  DynamicJsonBuffer jsonBufferDevices;
  JsonObject* rootDevices = &jsonBufferDevices.parseObject(devicesFile);

  if((*rootDevices)==JsonObject::invalid()) {
    rootDevices=&jsonBufferDevices.createObject();
  }

  for(auto kv : (*rootDevices))
  {
    myLogger.addToLog("adding device: ",kv.key);
    alexa.addDevice(kv.key);
    alexaEnabled=true;
  }
  devicesFile.close();

  if(alexaEnabled) {
    alexa.enable(true);

    alexa.onSetState([this](unsigned char device_id, const char * device_name, bool state) {
        myLogger.addToLog("Device: ",String(device_name));
        myLogger.addToLog("State: ",String(state));
        sendIR(String(device_name),state);
    });
    alexa.onGetState([](unsigned char device_id, const char * device_name) {
        return true; // whatever the state of the device is
    });
  }
}

void IRBridge::handleSendIR(AsyncWebServerRequest *request)
{
  if((!request->hasArg("bookmark") && !(request->hasArg("device") && request->hasArg("status"))) || (request->hasArg("bookmark") && request->hasArg("device"))) {
          request->send(400);
          return;
  }

  if(request->hasArg("bookmark"))
  { // send IR command for bookmark
    if(!bookmarkExists(request->arg("bookmark")))
    {
      request->send(404);
      return;
    }

    File bookmarkFile=SPIFFS.open(bookmarksprefix+request->arg("bookmark"),"r");

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(bookmarkFile);

    if(root==JsonObject::invalid()) {
            // error parsing config file, falling back to defaults
            myLogger.addToLog("Error loading bookmark ",request->arg("bookmark"));
            bookmarkFile.close();
            request->send(500);
            return;
    }

    if(sendIR(root))
    {
      request->send(200);
      bookmarkFile.close();
      return;

    } else {
      request->send(500);
      bookmarkFile.close();
      return;
    }
  } else if(request->hasArg("device"))
  { // send IR command for device
    File devicesFile=SPIFFS.open(devicesfilename,"r");

    DynamicJsonBuffer jsonBufferDevices;
    JsonObject* rootDevices = &jsonBufferDevices.parseObject(devicesFile);

    if((*rootDevices)==JsonObject::invalid()) {
      rootDevices=&jsonBufferDevices.createObject();
    }

    if(!rootDevices->containsKey(request->arg("device"))) {
      request->send(404);
      devicesFile.close();
      return;
    }

    if(!(request->arg("status").equalsIgnoreCase("on") || request->arg("status").equalsIgnoreCase("off")))
    {
      request->send(400);
      devicesFile.close();
      return;
    }

    if(sendIR(request->arg("device"),(request->arg("status").equalsIgnoreCase("on") ? true : false)))
    {
      request->send(200);
      devicesFile.close();
      return;
    } else {
      request->send(500);
      devicesFile.close();
      return;
    }
  }
}

bool IRBridge::sendIR(String devicename, bool status)
{
  myLogger.addToLog("Sending device: ",devicename);
  myLogger.addToLog("Status: ",String(status));

  File devicesFile=SPIFFS.open(devicesfilename,"r");

  DynamicJsonBuffer jsonBufferDevices;
  JsonObject* rootDevices = &jsonBufferDevices.parseObject(devicesFile);

  if((*rootDevices)==JsonObject::invalid()) {
    rootDevices=&jsonBufferDevices.createObject();
  }

  if(!rootDevices->containsKey(devicename) || (status && !(*rootDevices)[devicename].as<JsonObject>().containsKey("on")) || (!status && !(*rootDevices)[devicename].as<JsonObject>().containsKey("off"))) {
    devicesFile.close();
    return false;
  }

  JsonArray& deviceIRactions=(*rootDevices)[devicename][(status ? "on" : "off")].as<JsonArray>();

  for(auto item : deviceIRactions) {
    JsonObject& action=item.as<JsonObject>();

    if(action.containsKey("bookmark")) {
      File bookmarkFile=SPIFFS.open(bookmarksprefix+action["bookmark"].as<String>(),"r");

      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(bookmarkFile);

      if(root==JsonObject::invalid()) {
              // error parsing config file, falling back to defaults
              myLogger.addToLog("Error loading bookmark ",action["bookmark"].as<String>());
              bookmarkFile.close();
              return false;
      }
      if(!sendIR(root)) {
        bookmarkFile.close();
        return false;
      }
      bookmarkFile.close();
    } else if(action.containsKey("pause")) {
      myLogger.addToLog("sending pause...");

      uint32_t duration=action["pause"].as<unsigned long>();

      irqueue.add(std::make_shared<IRPause>(IRPause(duration)));
    }
  }
  devicesFile.close();

  return true;
}

bool IRBridge::sendIR(ArduinoJson::JsonObject &root)
{
  if(!validateCommandJson(root))
  {
    return false;
  }

  myLogger.addToLog("sendIR");

  if(root["type"].as<String>().equalsIgnoreCase("P"))
  {
    int ir_type=root["content"]["prot"];
    uint64_t code=root["content"]["value"].as<long long>();
    uint16_t bits=root["content"].as<JsonObject>().containsKey("bits") ? root["content"]["bits"] : 0;
    uint16_t repeat=root.containsKey("repeats") ? root["repeats"] : 0;

    myLogger.addToLog("adding queue element (protocol)");
    irqueue.add(std::make_shared<IRProtocol>(IRProtocol(ir_type,code,bits,repeat)));

  } else if(root["type"].as<String>().equalsIgnoreCase("R"))
  {
    myLogger.addToLog("sending raw...");

    std::vector<uint16_t> rawvalues(root["content"]["raw"].as<JsonArray>().size());
    root["content"]["raw"].as<JsonArray>().copyTo(rawvalues.data(),root["content"]["raw"].as<JsonArray>().size());

    uint16_t freq=root["content"].as<JsonObject>().containsKey("freq") ? root["content"]["freq"] : 0;
    uint16_t repeat=root.containsKey("repeats") ? root["repeats"] : 0;

    irqueue.add(std::make_shared<IRRaw>(IRRaw(rawvalues,freq,repeat)));
  }

  return true;
}

bool IRBridge::validateBookmarkJson(JsonObject& root) {
  // validate required parameters
  if(!root.containsKey("name") || !root["name"].is<char*>() || !root["name"].as<String>().length()>0) {
    return false;
  }

  return validateCommandJson(root);
}

bool IRBridge::validateCommandJson(JsonObject& root) {
  if(!root.containsKey("type") || (!(root["type"].as<String>().equalsIgnoreCase("P")) && !(root["type"].as<String>().equalsIgnoreCase("R")))) {
    return false;
  }

  if(!root.containsKey("content") || !root["content"].is<JsonObject>()) {
    return false;
  }

  if(root.containsKey("repeats") && !root["repeats"].is<unsigned int>()) {
    return false;
  }

  JsonObject& content=root["content"].as<JsonObject>();
  if(root["type"].as<String>().equalsIgnoreCase("P")) {
    if(!content.containsKey("prot") || !content.containsKey("value")) {
      return false;
    }

    if(!content["prot"].is<unsigned int>() || !content["value"].is<unsigned long>()) {
      return false;
    }

    if(content.containsKey("bits") && !content["bits"].is<unsigned int>()) {
      return false;
    }
  } else if(root["type"].as<String>().equalsIgnoreCase("R")) {
    if(!content.containsKey("raw") || !content["raw"].is<JsonArray>()) {
      return false;
    }

    if(content.containsKey("freq") && !content["freq"].is<unsigned long>()) {
      return false;
    }

    if(content.containsKey("enc") && ((!(content["enc"].as<String>().equalsIgnoreCase("D")) && !(content["enc"].as<String>().equalsIgnoreCase("H"))))) {
      return false;
    }

    JsonArray& raw=content["raw"].as<JsonArray>();
    for(int i=0; i<raw.size(); i++) {
      if(!raw[i].is<unsigned long>()) {
        return false;
      }
    }
  }

  return true;
}

bool IRBridge::validateDeviceJson(JsonObject& root) {
  if(!root.containsKey("name") || !root["name"].is<char*>() || !root["name"].as<String>().length()>0) {
    myLogger.addToLog("validateDeviceJson: Error in name.");
    return false;
  }

  if(!root.containsKey("actions") || !root["actions"].is<JsonObject>()) {
    myLogger.addToLog("validateDeviceJson: Error in actions.");
    return false;
  }

  JsonObject& actions=root["actions"].as<JsonObject>();
  for(auto kv : actions) {
    if(!String(kv.key).equalsIgnoreCase("on") && !String(kv.key).equalsIgnoreCase("off")) {
      myLogger.addToLog("validateDeviceJson: Not on or off.");
      return false;
    }
    if(!kv.value.is<JsonArray>()) {
      myLogger.addToLog("validateDeviceJson: Not an array.");
      return false;
    }

    JsonArray& actionsarray=kv.value.as<JsonArray>();
    for(int i=0; i<actionsarray.size(); i++) {
      if(!actionsarray[i].is<JsonObject>()) {
        myLogger.addToLog("validateDeviceJson: Not an object.");
        return false;
      }
      JsonObject& action=actionsarray[i];
      if(action.containsKey("bookmark")) {
        if(!action["bookmark"].is<char*>() || !action["bookmark"].as<String>().length()>0) {
          return false;
        }
      } else if(action.containsKey("pause")) {
        if(!action["pause"].is<unsigned long>()) {
          return false;
        }
      } else {
        //return false;
      }
    }
  }

  return true;
}

void IRBridge::whitelistBookmarkJson(JsonObject& root, std::vector<String> whitelist) {
  // loop through objects in root
  for( const auto& kv : root ) {
    // and check if object found in whitelist
    if(std::find(std::begin(whitelist), std::end(whitelist), kv.key)==std::end(whitelist)) {
      // key not found in whitelist, thus delete
      root.remove(kv.key);
    }
  }
}

bool IRBridge::bookmarkExists(String bookmarkName)
{
  return SPIFFS.exists(bookmarksprefix+bookmarkName);
}

String IRBridge::bookmarkUsedInDevice(String bookmarkname)
{
  File devicesFile=SPIFFS.open(devicesfilename,"r");

  DynamicJsonBuffer jsonBufferDevices;
  JsonObject* rootDevices = &jsonBufferDevices.parseObject(devicesFile);

  if((*rootDevices)==JsonObject::invalid()) {
    rootDevices=&jsonBufferDevices.createObject();
  }

  for(auto dev : (*rootDevices)) {
    JsonObject& device=dev.value.as<JsonObject>();
    for(auto sta : device) {
      for(auto act : sta.value.as<JsonArray>()) {
        JsonObject& action=act.as<JsonObject>();
        if(action.containsKey("bookmark") && action["bookmark"].as<String>().equalsIgnoreCase(bookmarkname)) {
          return String(dev.key);
        }
      }
    }
  }

  return "";
}

void IRBridge::propagateBookmarkRename(String oldBookmarkName, String newBookmarkName)
{
  File devicesFile=SPIFFS.open(devicesfilename,"r");

  DynamicJsonBuffer jsonBufferDevices;
  JsonObject* rootDevices = &jsonBufferDevices.parseObject(devicesFile);

  if((*rootDevices)==JsonObject::invalid()) {
    rootDevices=&jsonBufferDevices.createObject();
  }

  for(auto dev : (*rootDevices)) {
    JsonObject& device=dev.value.as<JsonObject>();
    for(auto sta : device) {
      for(auto act : sta.value.as<JsonArray>()) {
        JsonObject& action=act.as<JsonObject>();
        if(action.containsKey("bookmark") && action["bookmark"].as<String>().equalsIgnoreCase(oldBookmarkName)) {
          action["bookmark"]=newBookmarkName;
        }
      }
    }
  }

  // TODO:: randomize hardcoded temporary filenames to make sure concurrent requests from different users don't collide
  File outputFile=SPIFFS.open("/tempdev.dat","w");
  rootDevices->prettyPrintTo(outputFile);

  outputFile.close();
  devicesFile.close();

  SPIFFS.remove(devicesfilename);
  SPIFFS.rename("/tempdev.dat",devicesfilename);
}

bool IRBridge::bookmarkLengthOk(String bookmarkname)
{
  if((bookmarksprefix+bookmarkname).length()<=31) return true;
  else return false;
}

void IRBridge::handleGetBookmarks(AsyncWebServerRequest *request)
{
  if(request->hasArg("short")) {
    AsyncJsonResponse * response = new AsyncJsonResponse(true);
    JsonArray& root = response->getRoot();

    Dir myDir=SPIFFS.openDir(bookmarksprefix);
    while(myDir.next()) {
      root.add(String(myDir.fileName()).substring(bookmarksprefix.length()));
    }

    response->setLength();
    request->send(response);
  } else {
    if(request->_tempObject==nullptr) {
      std::vector<String> includepaths;
      includepaths.push_back(bookmarksprefix);
      std::vector<String> filesblacklist;
      request->_tempObject=new FilesToJSON(includepaths,filesblacklist,false);
    }

    request->onDisconnect([request]() {
      delete ((FilesToJSON*)request->_tempObject);
      request->_tempObject=nullptr;
    });

    AsyncWebServerResponse *response=request->beginChunkedResponse("application/json", [request](uint8_t *buffer, size_t maxlen, size_t index) -> size_t {
      size_t bytesWritten=((FilesToJSON*)request->_tempObject)->continueFiles(buffer, maxlen);
      myLogger.addToLog("ChunkedResponse: ",String(bytesWritten));

      return bytesWritten;
    });

    request->send(response);
  }

  return;
}

void IRBridge::handleGetDevices(AsyncWebServerRequest *request)
{
  if(SPIFFS.exists(devicesfilename)) {
    request->send(SPIFFS, devicesfilename,"application/json");
  } else {
    request->send(200,"application/json", "{}");
  }

  return;
}

// POST on /api/bookmarks for adding new bookmarks. return 409 if name already exists, 400 if required parameter is missing
void IRBridge::handleAddBookmark(AsyncWebServerRequest *request)
{
  if(request->method()!=HTTP_POST || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
          request->send(400);
          return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)request->_tempObject);

  // check if valid JSON file
  if(root==JsonObject::invalid() || !validateBookmarkJson(root)) {
          // error parsing config file
          request->send(400);
          return;
  }

  if(!bookmarkLengthOk(root["name"])) {
    request->send(400);
    return;
  }

  whitelistBookmarkJson(root, {"name","type","repeats","content"});

  if(root["type"].as<String>().equalsIgnoreCase("P")) whitelistBookmarkJson(root["content"].as<JsonObject>(), {"prot","value","bits"});
  else if(root["type"].as<String>().equalsIgnoreCase("R")) whitelistBookmarkJson(root["content"].as<JsonObject>(), {"raw","enc","freq"});

  if(bookmarkExists(root["name"])) {
    request->send(409);
    return;
  }

  File f=SPIFFS.open(bookmarksprefix+root["name"].as<String>(),"w");

  if(!f) {
    request->send(500);
    return;
  } else {
    root.prettyPrintTo(f);
    request->send(200);
    return;
  }
}

// POST on /api/devices for adding new devices.
// return 409 if name already exists,
// 400 if required parameter is missing,
// 507 if already max. number of devices reached (16)
void IRBridge::handleAddDevice(AsyncWebServerRequest *request)
{
  if(request->method()!=HTTP_POST || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
          request->send(400);
          return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)request->_tempObject);

  // check if valid JSON file
  if(root==JsonObject::invalid() || !validateDeviceJson(root)) {
    // error parsing device json
    request->send(400);
    return;
  }

  File devicesFile=SPIFFS.open(devicesfilename,"r");

  DynamicJsonBuffer jsonBufferDevices;
  JsonObject* rootDevices = &jsonBufferDevices.parseObject(devicesFile);

  if((*rootDevices)==JsonObject::invalid()) {
    rootDevices=&jsonBufferDevices.createObject();
  }

  if(rootDevices->size()>=16) {
    // can only store 16 devices at a time
    request->send(507);
    devicesFile.close();
    return;
  }

  if(rootDevices->containsKey(root["name"].as<String>())) {
//  for(auto dev : (*rootDevices)) {
//    if(dev.key==root["name"]) {
      request->send(409);
      devicesFile.close();
      return;
//    }
  }

  JsonObject& newDevice=rootDevices->createNestedObject(root["name"].as<String>());
  for(auto kv : root["actions"].as<JsonObject>()) {
    JsonArray& newActions=newDevice.createNestedArray(kv.key);
    for(auto act : kv.value.as<JsonArray>()) {
      JsonObject& action=act.as<JsonObject>();
      JsonObject& newAction=newActions.createNestedObject();

      if(action.containsKey("bookmark")) {
        newAction["bookmark"]=action["bookmark"];
      } else if(action.containsKey("pause")) {
        newAction["pause"]=action["pause"];
      }
    }
  }

  File outputFile=SPIFFS.open("/tempdev.dat","w");
  rootDevices->prettyPrintTo(outputFile);

  outputFile.close();
  devicesFile.close();

  SPIFFS.remove(devicesfilename);
  SPIFFS.rename("/tempdev.dat",devicesfilename);

  request->send(200);
}

// PUT on /api/bookmarks/[name] for editing bookmark [name]. Can also change [name]. return 409 if name already exists, 400 if required parameter is missing
void IRBridge::handleEditBookmark(AsyncWebServerRequest *request, String bookmarkname)
{
  if(request->method()!=HTTP_PUT || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
          request->send(400);
          return;
  }

  if(!bookmarkLengthOk(bookmarkname)) {
    request->send(400);
    return;
  }

  if(!bookmarkExists(bookmarkname)) {
    request->send(404);
    return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)request->_tempObject);

  // check if valid JSON file
  if(root==JsonObject::invalid() || !validateBookmarkJson(root)) {
          // error parsing config file
          request->send(400);
          return;
  }

  if(!bookmarkLengthOk(root["name"])) {
    request->send(400);
    return;
  }

  whitelistBookmarkJson(root, {"name","type","repeats","content"});

  if(root["type"].as<String>().equalsIgnoreCase("P")) whitelistBookmarkJson(root["content"].as<JsonObject>(), {"prot","value","bits"});
  else if(root["type"].as<String>().equalsIgnoreCase("R")) whitelistBookmarkJson(root["content"].as<JsonObject>(), {"raw","enc","freq"});

  if(bookmarkExists(root["name"]) && bookmarkname!=root["name"].as<String>()) {
    request->send(409);
    return;
  }

  if(!SPIFFS.remove(bookmarksprefix+bookmarkname)) {
    request->send(500);
    return;
  }

  File f=SPIFFS.open(bookmarksprefix+root["name"].as<String>(),"w");

  if(!f) {
    request->send(500);
    return;
  } else {
    root.prettyPrintTo(f);

    if(bookmarkname!=root["name"].as<String>()) {
      propagateBookmarkRename(bookmarkname,root["name"].as<String>());
    }

    request->send(200);
    return;
  }
}

// PUT on /api/devices/[name] for editing device [name].
// Can also change [name].
// return 409 if name already exists,
// 400 if required parameter is missing
void IRBridge::handleEditDevice(AsyncWebServerRequest *request, String devicename)
{
  if(request->method()!=HTTP_PUT || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
          request->send(400);
          return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)request->_tempObject);

  // check if valid JSON file
  if(root==JsonObject::invalid() || !validateDeviceJson(root)) {
    // error parsing device json
    request->send(400);
    return;
  }

  File devicesFile=SPIFFS.open(devicesfilename,"r");

  DynamicJsonBuffer jsonBufferDevices;
  JsonObject* rootDevices = &jsonBufferDevices.parseObject(devicesFile);

  if((*rootDevices)==JsonObject::invalid()) {
    rootDevices=&jsonBufferDevices.createObject();
  }

  if(!rootDevices->containsKey(devicename)) {
    // device not found
    request->send(404);
    devicesFile.close();
    return;
  } else if(rootDevices->containsKey(root["name"].as<String>()) && root["name"].as<String>()!=devicename) {
    // a device should be renamed or added, but the target device already exists
    request->send(409);
    devicesFile.close();
    return;
  }

  // remove the old devicename
  rootDevices->remove(devicename);

  JsonObject& newDevice=rootDevices->createNestedObject(root["name"].as<String>());
  for(auto kv : root["actions"].as<JsonObject>()) {
    JsonArray& newActions=newDevice.createNestedArray(kv.key);
    for(auto act : kv.value.as<JsonArray>()) {
      JsonObject& action=act.as<JsonObject>();
      JsonObject& newAction=newActions.createNestedObject();

      if(action.containsKey("bookmark")) {
        newAction["bookmark"]=action["bookmark"];
      } else if(action.containsKey("pause")) {
        newAction["pause"]=action["pause"];
      }
    }
  }

  File outputFile=SPIFFS.open("/tempdev.dat","w");
  rootDevices->prettyPrintTo(outputFile);

  outputFile.close();
  devicesFile.close();

  SPIFFS.remove(devicesfilename);
  SPIFFS.rename("/tempdev.dat",devicesfilename);

  request->send(200);
}

// DELETE on /api/bookmarks/[name]. return 404 if bookmark can't be found
// 423 if bookmark is used in a device
void IRBridge::handleDeleteBookmark(AsyncWebServerRequest *request, String bookmarkname)
{
  if(!bookmarkLengthOk(bookmarkname)) {
    request->send(400);
    return;
  }

  String device=bookmarkUsedInDevice(bookmarkname);
  if(device!="") {
    request->send(423, "text/plain", device);
    return;
  }

  if(bookmarkExists(bookmarkname)) {
    if(SPIFFS.remove(bookmarksprefix+bookmarkname)) {
      request->send(200);
      return;
    } else {
      request->send(500);
      return;
    }
  }

  request->send(404);
  return;
}

// DELETE on /api/devices/[name]. return 404 if device can't be found
void IRBridge::handleDeleteDevice(AsyncWebServerRequest * request, String devicename)
{
  File devicesFile=SPIFFS.open(devicesfilename,"r");

  DynamicJsonBuffer jsonBufferDevices;
  JsonObject& rootDevices = jsonBufferDevices.parseObject(devicesFile);

  if(rootDevices==JsonObject::invalid()) {
    request->send(404);
    return;
  }

  if(rootDevices.containsKey(devicename)) {
    rootDevices.remove(devicename);
  } else {
    request->send(404);
    devicesFile.close();
    return;
  }

  File outputFile=SPIFFS.open("/tempdev.dat","w");
  rootDevices.prettyPrintTo(outputFile);

  outputFile.close();
  devicesFile.close();

  SPIFFS.remove(devicesfilename);
  SPIFFS.rename("/tempdev.dat",devicesfilename);

  request->send(200);
}
