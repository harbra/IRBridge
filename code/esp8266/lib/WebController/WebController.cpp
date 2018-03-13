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

#include "WebController.h"

WebController::WebController() : server(80), pathprefix("/html"), events("/events"), previousMillisConnectionCheck(0), timeoutConnectionCheck(5000), resetWifi(false), reboot(false), restoreSettingsFile("")
{
}

void WebController::setup()
{
        myLogger.addToLog("Starting...");
        SPIFFS.begin();
        SPIFFS.remove("/temp/temp");

        settingsfilename="/config/webserver.json";
        loadSettings(settingsfilename);
        resetWifiConfig();

        resetSettingsIncludePaths();
        resetSettingsBlacklist();

        setupServer();

        WiFi.scanNetworks(true);
}

void WebController::loop()
{
        dnsserver.processNextRequest();

        if(resetWifi)
        {
          resetWifiConfig();
          resetWifi=false;
        }

        if(restoreSettingsFile!="")
        {
          restoreSettings();
        }

        if(reboot)
        {
          reboot=false;
          delay(2500);
          ESP.restart();
        }

        if(millis()-previousMillisConnectionCheck>=timeoutConnectionCheck)
        {
          previousMillisConnectionCheck=millis();
          sendEvent("ConnectionCheck",getConnectionStatus());
        }
}

String WebController::getConnectionStatus()
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["isConnected"]=WiFi.isConnected();
  if(WiFi.isConnected())
  {
      root["RSSI"]=WiFi.RSSI();
      root["SSID"]=WiFi.SSID();
      if(WiFi.psk().length()>0)
      {
        root["encrypted"]=true;
      } else
      {
        root["encrypted"]=false;
      }
  }

  String output;
  root.printTo(output);

  return output;
}

void WebController::sendEvent(String eventname, String eventcontent)
{
  myLogger.addToLog("SendEvent: ",eventcontent);
  events.send(eventcontent.c_str(),eventname.c_str());
}

void WebController::setupServer()
{
      // set up html server
        myLogger.addToLog("Setting up server...");
        server.on("/api/test",HTTP_POST,std::bind(&WebController::handleTest, this, std::placeholders::_1),std::bind(&WebController::handleFileUploadDummy, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6),std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        server.on("/api/settings",HTTP_GET,std::bind(&WebController::handleGetSettings, this, std::placeholders::_1));
        server.on("/api/settings",HTTP_POST,std::bind(&WebController::handleSetSettings, this, std::placeholders::_1),std::bind(&WebController::handleFileUploadDummy, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6),std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        server.on("/api/scanwifi",HTTP_GET,std::bind(&WebController::handleScanWifi, this, std::placeholders::_1));
        server.on("/api/getinfo",HTTP_GET,std::bind(&WebController::handleGetInfo, this, std::placeholders::_1));
        server.on("/api/getwifistatus",HTTP_GET,std::bind(&WebController::handleGetWifiStatus, this, std::placeholders::_1));
        server.on("/api/reboot",HTTP_GET,std::bind(&WebController::handleReboot, this, std::placeholders::_1));
        server.on("/api/downloadsettings",HTTP_GET,std::bind(&WebController::handleDownloadSettings, this, std::placeholders::_1));
        server.on("/api/restoresettings",HTTP_POST,std::bind(&WebController::handleRestoreSettings, this, std::placeholders::_1),std::bind(&WebController::handleFileUpload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6),std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

        // server content from SPIFFS in case no route already caught this URI
        server.onNotFound([this](AsyncWebServerRequest *request){
                                  bool handled=false;
                                  for(WebHandlerContainer myWebHandler : additionalWebHandlers) {
                                    if(request->url().startsWith(myWebHandler.myUri) && request->method()==myWebHandler.myMethod) {
                                      handled=true;
                                      myWebHandler.myOnRequest(request,request->url().substring(myWebHandler.myUri.length()));
                                    }
                                  }
                                  if(!handled) {
                                    if(request->url().startsWith("/api")) {
                                      // sepcial processing for api requests
                                      request->send(404);
                                    } else if(!handleFileRead(request, request->url()))
                                    {
                                        handleFileRead(request, "/");  // in case no file with this URI is found, return the standard index.html file. Needed for vue.js in history mode.
                                    }
                                  }
                          });

        server.onRequestBody(std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

        // set up server-sent events server
        events.onConnect([](AsyncEventSourceClient *client){
          if(client->lastId()){
            // client reconnected
          }
          //client->send("Hello!",NULL,millis(),1000);
        });

        server.addHandler(&events);

        server.begin();
}

void WebController::resetWifiConfig()
{
  if(!connectWifi()) {
    myLogger.addToLog("Couldn't connect to an existing AP. Setting one up.");
      createAP();
  }
  if(!MDNS.begin(ap_ssid)) {
    myLogger.addToLog("Couldn't activate MDNS responder.");
  } else {
    myLogger.addToLog("MDNS responder activated at: ",String(ap_ssid));
  }
}

bool WebController::connectWifi()
{
        dnsserver.stop();

        WiFi.mode(WIFI_STA);

        if(!sta_dhcp) {
          IPAddress IP_sta_ip; IPAddress IP_sta_gw; IPAddress IP_sta_sn;

          if(IP_sta_ip.fromString(sta_ip) &&
             IP_sta_gw.fromString(sta_gw) &&
             IP_sta_sn.fromString(sta_sn)) {
               WiFi.config(IP_sta_ip,IP_sta_gw,IP_sta_sn);
             } else {
               myLogger.addToLog("Invalid static IP configuration. Using DHCP.");
             }
        }

        if(WiFi.status()==WL_CONNECTED) {
          myLogger.addToLog("Connected.");
          return true;
        }

        if(sta_ssid==NULL || strlen(sta_ssid)==0)
        {
          myLogger.addToLog("No SSID given for AP. Cancelling.");
          return false;
        }

        if(strlen(sta_pwd)>=8 && strlen(sta_pwd)<=63) {
          myLogger.addToLog("Connecting Wifi. SSID: ", sta_ssid);
          myLogger.addToLog("PW: ", sta_pwd);
            WiFi.begin(sta_ssid, sta_pwd);
        } else {
          myLogger.addToLog("Connecting Wifi without password. SSID: ", sta_ssid);
          WiFi.begin(sta_ssid);
        }

        if(WiFi.waitForConnectResult() == WL_CONNECTED) {
          myLogger.addToLog("Connected.");
          return true;
        } else {
          myLogger.addToLog("Failed to connect.");
          return false;
        }
}

void WebController::createAP()
{
        myLogger.addToLog("Setting up access point...");
        WiFi.mode(WIFI_AP_STA);
        //WiFi.softAPConfig(IPAddress(192,168,1,1),IPAddress(192,168,1,1),IPAddress(255,255,255,0));
        WiFi.softAP(ap_ssid,ap_pwd);

        dnsserver.setErrorReplyCode(DNSReplyCode::NoError);
        dnsserver.start(53,"*",IPAddress(192,168,4,1));
}

String WebController::getContentType(String filename)
{
        if(filename.endsWith(".htm")) return "text/html";
        else if(filename.endsWith(".html")) return "text/html";
        else if(filename.endsWith(".css")) return "text/css";
        else if(filename.endsWith(".js")) return "application/javascript";
        else if(filename.endsWith(".png")) return "image/png";
        else if(filename.endsWith(".gif")) return "image/gif";
        else if(filename.endsWith(".jpg")) return "image/jpeg";
        else if(filename.endsWith(".ico")) return "image/x-icon";
        else if(filename.endsWith(".xml")) return "text/xml";
        else if(filename.endsWith(".pdf")) return "application/x-pdf";
        else if(filename.endsWith(".zip")) return "application/x-zip";
        else if(filename.endsWith(".gz")) return "application/gzip";
        return "text/plain";
}

void WebController::handleReboot(AsyncWebServerRequest * request)
{
  reboot=true;
}

void WebController::handleGetWifiStatus(AsyncWebServerRequest * request)
{
  request->send(200,"application/json",getConnectionStatus());
}

void WebController::handleRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  if(request->_tempObject==nullptr) {
    request->_tempObject=malloc(total+1);
    ((uint8_t*)request->_tempObject)[total]='\0';
  }

  for(size_t i=0; i<len; i++){
    ((uint8_t*)request->_tempObject)[index+i]=data[i];
  }
}

void WebController::handleFileUploadDummy(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
}

void WebController::handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  /*if(String(request->_tempFile.name()).equalsIgnoreCase("")) {
    request->_tempFile=SPIFFS.open("/temp/"+filename,"w");
  } else if(!String(request->_tempFile.name()).endsWith("/"+filename)) {
    request->send(422);
  }*/
  File myFile=SPIFFS.open("/temp/"+filename,"a");

  myLogger.addToLog("Writing to file: ",myFile.name());

  myLogger.addToLog("seek: ",String(std::min(myFile.size(),index)));
  myFile.seek(std::min(myFile.size(),index));
  myLogger.addToLog("position: ",String(myFile.position()));
  myLogger.addToLog("index: ",String(index));

  while(myFile.position()<index) {
    myFile.write(0);
  }

  myLogger.addToLog("length: ",String(len));
  myLogger.addToLog("written: ",String(myFile.write(data,len)));

  //if(final) {
    myFile.close();
  /*} else {
    request->_tempFile.flush();
  }*/
}

void WebController::handleDownloadSettings(AsyncWebServerRequest * request)
{
  if(request->_tempObject==nullptr) {
    request->_tempObject=new FilesToJSON(settings_includepaths,settings_filesblacklist);
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

void WebController::handleRestoreSettings(AsyncWebServerRequest * request)
{
  // check if a FILE was posted
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile() && p->value()!=""){
      // TODO: Check if filenames are longer than 31 characters and do something if yes (not only here)
      myLogger.addToLog("Filename for restore: ",p->value());
      restoreSettingsFile="/temp/"+p->value();
      request->send(200);
    }
  }

  request->send(400);
}

void WebController::handleGetInfo(AsyncWebServerRequest *request)
{
  AsyncJsonResponse * response = new AsyncJsonResponse(false);
  //DynamicJsonBuffer jsonOutput;
  JsonObject& output=response->getRoot();

  output["chipid"]=ESP.getChipId();
  if(WiFi.isConnected()) {
    output["ip"]=WiFi.localIP().toString();
  } else {
    output["ip"]=WiFi.softAPIP().toString();
  }

  if(request->hasArg("comprehensive")) {
    if(WiFi.isConnected()) {
      output["isconnected"]=true;
      output["bssid"]=WiFi.BSSID();
      output["channel"]=WiFi.channel();
      output["gatewayip"]=WiFi.gatewayIP().toString();
      output["hostname"]=WiFi.hostname();
      output["macaddress"]=WiFi.macAddress();
      output["ssid"]=WiFi.SSID();
      output["subnetmask"]=WiFi.subnetMask().toString();
      if(WiFi.psk().length()>0)
      {
        output["encrypted"]=true;
      } else
      {
        output["encrypted"]=false;
      }
    } else {
      output["isconnected"]=false;
      output["softapip"]=WiFi.softAPIP().toString();
      output["softapmac"]=WiFi.softAPmacAddress();
    }
    output["cpufreqmhz"]=ESP.getCpuFreqMHz();
    output["flashchipid"]=ESP.getFlashChipId();
    output["flashchipsize"]=ESP.getFlashChipSize();
    output["flashchiprealsize"]=ESP.getFlashChipRealSize();
    output["flashchipspeed"]=ESP.getFlashChipSpeed();
    output["freeheap"]=ESP.getFreeHeap();
    output["freesketchspace"]=ESP.getFreeSketchSpace();
    output["sketchsize"]=ESP.getSketchSize();
    output["millis"]=millis();
  }

  response->setLength();
  request->send(response);
}

void WebController::handleTest(AsyncWebServerRequest *request)
{
        myLogger.addToLog("url: ",String(request->url()));
        myLogger.addToLog("method: ", String(request->method()));
        myLogger.addToLog("headers: ", String(request->headers()));
        for (uint8_t i=0; i<request->headers(); i++) {
                myLogger.addToLog(request->headerName(i) + ": ",String(request->header(i)));
        }
        myLogger.addToLog("args: ", String(request->args()));
        for (uint8_t i=0; i<request->args(); i++) {
                myLogger.addToLog(request->argName(i) + ": ",String(request->arg(i)));
        }
        if(request->_tempObject!=nullptr)
        {
          myLogger.addToLog("body: ",String((char*)request->_tempObject));

          // memory is freed automatically in the request's destructor
          //free(request->_tempObject);
          //request->_tempObject=nullptr;
        }
        //List all parameters
        int params = request->params();
        for(int i=0;i<params;i++){
          AsyncWebParameter* p = request->getParam(i);
          if(p->isFile()){ //p->isPost() is also true
            Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
          } else if(p->isPost()){
            Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
          } else {
            Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
          }
        }
        request->send(204);
}

bool WebController::handleScanWifi(AsyncWebServerRequest *request)
{
        //int n = WiFi.scanNetworks();
        //myLogger.addToLog("Found networks: ",String(n));

        AsyncJsonResponse * response = new AsyncJsonResponse(true);
        //DynamicJsonBuffer jsonOutput;
        JsonArray& output=response->getRoot();

        int n = WiFi.scanComplete();
        if(n == -2){
          WiFi.scanNetworks(true);
        } else if(n){
          myLogger.addToLog("Found networks: ",String(n));
                for (int i = 0; i < n; ++i)
                {
                    JsonObject& output_network=output.createNestedObject();
                    output_network["ssid"]=WiFi.SSID(i);
                    output_network["rssi"]=WiFi.RSSI(i);
                    output_network["enc"]=WiFi.encryptionType(i);
                    output_network["channel"]=WiFi.channel(i);
                }
                WiFi.scanDelete();
                if(WiFi.scanComplete()==-2){
                  WiFi.scanNetworks(true);
                }

        }

        //String buffer;
        //output.prettyPrintTo(buffer);
        response->setLength();
        request->send(response);

        return true;
}

bool WebController::handleSetSettings(AsyncWebServerRequest *request)
{
        if(request->method()!=HTTP_POST || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
                request->send(400);
                return false;
        }

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject((char*)request->_tempObject);

        if(root==JsonObject::invalid()) {
                // error parsing config file
                request->send(400);
                return false;
        }

        File settingsFile=SPIFFS.open(settingsfilename,"r");

        DynamicJsonBuffer jsonBufferSettings;
        JsonObject& rootSettings = jsonBufferSettings.parseObject(settingsFile);

        if(rootSettings==JsonObject::invalid()) {
                // error parsing config file
                myLogger.addToLog("Can't read config.");
                request->send(500);
                settingsFile.close();

                return false;
        }
        JsonObject& settings=rootSettings["settings"];

        for(auto kt : root)
        {
                if(settings.containsKey(kt.key)) {
                        JsonObject& subsettings=settings[kt.key];

                        for(auto it : kt.value.as<JsonObject>()) {
                                if(subsettings.containsKey(it.key)) {
                                        myLogger.addToLog("Found: ",it.key);
                                        myLogger.addToLog("Writing: ",it.value.as<String>());
                                        JsonArray& subsettingsvalue=subsettings[it.key];
                                        subsettingsvalue[0]=it.value;
                                } else {
                                        myLogger.addToLog("Couldn't find subsetting: ", it.key);
                                }
                        }
                } else {
                        myLogger.addToLog("Couldn't find setting: ",kt.key);
                }
        }

        File outputFile=SPIFFS.open("/temp.dat","w");
        rootSettings.prettyPrintTo(outputFile);

        outputFile.close();
        settingsFile.close();

        SPIFFS.remove(settingsfilename);
        SPIFFS.rename("/temp.dat",settingsfilename);
        myLogger.addToLog("Settings saved.");

        request->send(200);

        loadSettings(settingsfilename);
        resetWifi=true;
}

bool WebController::handleGetSettings(AsyncWebServerRequest *request)
{
        if(request->method()!=HTTP_GET || !request->hasArg("type")) {
                myLogger.addToLog("Wrong HTTP method or server has no arg 'type'.");
                request->send(400);
                return false;
        }

        File settingsFile=SPIFFS.open(settingsfilename,"r");

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(settingsFile);

        if(root==JsonObject::invalid()) {
                // error parsing config file
                myLogger.addToLog("Can't read config.");
                request->send(500);
                settingsFile.close();
                return false;
        }
        JsonObject& settings=root["settings"];

        if(!settings.containsKey(request->arg("type")) && request->arg("type")!="*") {
                myLogger.addToLog("Couldn't find key: ",request->arg("type"));
                request->send(500);
                settingsFile.close();
                return false;
        }

        AsyncJsonResponse * response = new AsyncJsonResponse();

        JsonObject& output=response->getRoot();
        for(auto kt : settings)
        {
                if(request->arg("type")=="*" || request->arg("type")==kt.key)
                {
                        JsonObject& suboutput=output.createNestedObject(kt.key);
                        JsonObject& currentSettings=kt.value.as<JsonObject>();

                        for(auto it : currentSettings)
                        {
                                JsonArray& settingValue=it.value.as<JsonArray>();
                                if(settingValue.size()==1 || settingValue[1]==true)
                                { // the array either only has one element, or the second element is true, which means the value should be displayed
                                        suboutput[it.key]=settingValue[0];
                                }
                        }
                }
        }

        response->setLength();
        request->send(response);
        settingsFile.close();
        return true;
}

bool WebController::handleFileRead(AsyncWebServerRequest *request, String path)
{
        myLogger.addToLog("Requesting: ",path);
        path=pathprefix+path;
        if(path.endsWith("/")) path += "index.html";
        String contentType = getContentType(path);
        String pathWithGz = path + ".gz";
        if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
                if(SPIFFS.exists(pathWithGz))
                        path += ".gz";

                myLogger.addToLog("serving: ", path);

                AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);

                if(SPIFFS.exists(pathWithGz)) {
                  response->addHeader("Content-Encoding", "gzip");
                }

                request->send(response);

                return true;
        }
        myLogger.addToLog("404");
        return false;
}

bool WebController::loadSettings(String filename)
{
        myLogger.addToLog("Loading settings...");

        File settingsFile=SPIFFS.open(filename,"r");

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(settingsFile);

        if(root==JsonObject::invalid()) {
                // error parsing config file, falling back to defaults
                myLogger.addToLog("Error loading settings...");
                settingsFile.close();

                return false;
        }
        JsonObject& settings=root["settings"];

        strlcpy(ap_ssid,settings["ap"]["ssid"][0] | "",sizeof(ap_ssid));
        myLogger.addToLog("ap_ssid: ",ap_ssid);

        strlcpy(ap_pwd,settings["ap"]["pwd"][0] | "",sizeof(ap_pwd));
        myLogger.addToLog("ap_pwd: ",ap_pwd);

        strlcpy(sta_ssid,settings["sta"]["ssid"][0] | "",sizeof(sta_ssid));
        myLogger.addToLog("sta_ssid: ",sta_ssid);

        strlcpy(sta_pwd,settings["sta"]["pwd"][0] | "",sizeof(sta_pwd));
        myLogger.addToLog("sta_pwd: ",sta_pwd);

        sta_dhcp=settings["sta"]["dhcp"][0];
        myLogger.addToLog("sta_dhcp: ",String(sta_dhcp));

        strlcpy(sta_ip,settings["sta"]["ip"][0] | "",sizeof(sta_ip));
        myLogger.addToLog("sta_ip: ",sta_ip);

        strlcpy(sta_gw,settings["sta"]["gw"][0] | "",sizeof(sta_gw));
        myLogger.addToLog("sta_gw: ",sta_gw);

        strlcpy(sta_sn,settings["sta"]["sn"][0] | "",sizeof(sta_sn));
        myLogger.addToLog("sta_sn: ",sta_sn);

        // parsing seems to have been successful
        myLogger.addToLog("Done loading settings.");
        settingsFile.close();

        return true;
}

bool WebController::saveSettings(String filename)
{
        myLogger.addToLog("Saving settings...");

        File settingsFile=SPIFFS.open(filename,"r");

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(settingsFile);

        if(root==JsonObject::invalid()) {
                // error parsing config file
                myLogger.addToLog("Error loading settings...");
                settingsFile.close();
                return false;
        }

        JsonObject& settings=root["settings"];
        myLogger.addToLog("Modifying settings...");
        settings["ap"]["ssid"][0]=ap_ssid;
        settings["ap"]["pwd"][0]=ap_pwd;

        settings["sta"]["ssid"][0]=sta_ssid;
        settings["sta"]["pwd"][0]=sta_pwd;
        settings["sta"]["dhcp"][0]=sta_dhcp;
        settings["sta"]["ip"][0]=sta_ip;
        settings["sta"]["gw"][0]=sta_gw;
        settings["sta"]["sn"][0]=sta_sn;

        File outputFile=SPIFFS.open("/temp.dat","w");
        root.prettyPrintTo(outputFile);

        outputFile.close();
        settingsFile.close();

        SPIFFS.remove(filename);
        SPIFFS.rename("/temp.dat",filename);
        myLogger.addToLog("Settings saved.");
        // done updating settings
        return true;
}

std::pair<int, std::unique_ptr<char[]>> WebController::decode64(const char* data, size_t length)
{
    // decoded data is smaller than input.
    size_t size = base64_decode_expected_len(length)+1;
    //myLogger.addToLog("Size: ",String(size));
    std::unique_ptr<char[]> buffer(new char[size]);
    if(buffer) {
        //myLogger.addToLog("Starting decode");
        base64_decodestate _state;
        base64_init_decodestate(&_state);
        //myLogger.addToLog("Starting blocks");
        int len = base64_decode_block(data, length, buffer.get(), &_state);
        buffer.get()[len]=0;
        //myLogger.addToLog("Creating String");
        myLogger.addToLog("Calculated size: ",String(size));
        myLogger.addToLog("Real size: ",String(len));
        //String decoded = String(buffer);
        //myLogger.addToLog("Free heap: ",String(ESP.getFreeHeap()));
        //free(buffer);
        //myLogger.addToLog("freeing");
        return std::make_pair(len, std::move(buffer));
    }
    return std::make_pair(0, nullptr);
}

void WebController::restoreSettings()
{
  if(restoreSettingsFile=="") return;

  myLogger.addToLog("Restoring from : ",restoreSettingsFile);
  sendEvent("Restore","{\"type\":\"started\"}");
  File tempFile=SPIFFS.open(restoreSettingsFile,"r");
  restoreSettingsFile="";

  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray(tempFile);

  if(root==JsonArray::invalid()) {
          // error parsing uploaded config file, falling back to defaults
          myLogger.addToLog("Error parsing uploaded file.");
          sendEvent("Restore","{\"type\":\"failed\", \"reason\":\"Error parsing uploaded file.\"}");
          tempFile.close();
          return;
  }

  for(int i=0; i<root.size(); i++) {
    JsonObject& file=root[i];
    sendEvent("Restore","{\"type\":\"progress\", \"current\": "+String(i)+", \"total\": "+String(root.size())+"}");

    // iterate over all files in the uploaded data
    if(!(file.containsKey("filepath") && file.containsKey("content"))) {
      sendEvent("Restore","{\"type\":\"warning\", \"reason\":\"Found invalid JSON object with missing key filepath and/or content.\"}");
      continue;
    }

    // check for valid filename
    if(!file["filepath"].is<char*>() || file["filepath"].as<String>().length()>31 || file["filepath"].as<String>()=="") {
      sendEvent("Restore","{\"type\":\"warning\", \"reason\":\"Filepath \""+file["filepath"].as<String>()+"\" invalid.\"}");
      continue;
    }

    //sendEvent("Restore","Restoring \""+file["filepath"].as<String>()+"\".");
    sendEvent("Restore","{\"type\":\"filestatus\",\"filepath\":\""+file["filepath"].as<String>()+"\",\"status\":\"started\"}");

    // try to decode file contents
    //request->send(200,"text/plain",decode64((uint8_t*)file["content"].as<char*>(),file["content"].size()));
    //tempFile.close();
    std::pair<int,std::unique_ptr<char[]>> decoded=decode64(file["content"].as<char*>(),file["content"].as<String>().length());

    if(decoded.first==0 && decoded.second==nullptr) {
      sendEvent("Restore","{\"type\":\"filestatus\",\"filepath\":\""+file["filepath"].as<String>()+"\",\"status\":\"failed\",\"reason\":\"base64 decode failed.\"}");
      continue;
    } else {
      File myFile=SPIFFS.open(file["filepath"].as<String>(),"w");

      if(myFile) {
        myFile.write((uint8_t*) decoded.second.get(),decoded.first);
        sendEvent("Restore","{\"type\":\"filestatus\",\"filepath\":\""+file["filepath"].as<String>()+"\",\"status\":\"success\"}");
        myFile.close();
      } else {
        sendEvent("Restore","{\"type\":\"filestatus\",\"filepath\":\""+file["filepath"].as<String>()+"\",\"status\":\"failed\",\"reason\":\"Error opening file.\"}");
        continue;
      }
    }

    /*myLogger.addToLog("================================================================");
    myLogger.addToLog("Length: ",String(decoded.first));
    myLogger.addToLog(String(decoded.second.get()));
    myLogger.addToLog("================================================================");*/

    /*if(SPIFFS.exists(file["filepath"].as<String>())) {
      SPIFFS.remove(file["filepath"].as<String>());
    }*/
  }
  sendEvent("Restore","{\"type\":\"done\"}");

  reboot=true;
}

void WebController::addSettingsIncludePath(String path)
{
  settings_includepaths.push_back(path);
}

void WebController::resetSettingsIncludePaths()
{
  settings_includepaths.clear();
}

void WebController::addSettingsBlacklistFilename(String filename)
{
  settings_filesblacklist.push_back(filename);
}

void WebController::resetSettingsBlacklist()
{
  settings_filesblacklist.push_back("/config/webserver.json");
  settings_filesblacklist.clear();
}

void WebController::addServerHandler(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, bool acceptFileUpload, bool acceptBodyUpload)
{
  AsyncCallbackWebHandler* handler = new AsyncCallbackWebHandler();
  handler->setUri(uri);
  handler->setMethod(method);
  handler->onRequest(onRequest);
  if(acceptFileUpload) {
    handler->onUpload(std::bind(&WebController::handleFileUpload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  }
  if(acceptBodyUpload) {
    handler->onBody(std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
  }
  server.addHandler(handler);
}

void WebController::addServerHandler(const char* uri, WebRequestMethodComposite method, std::function<void(AsyncWebServerRequest *request, String parameters)> onRequest)
{
  additionalWebHandlers.push_back(WebHandlerContainer(uri,method,onRequest));
}
