# IRBridge - controlling IR devices via Alexa and webinterface

## Project description
The IRBridge can record and play infrared commands - configured by a webinterface, accessible via Alexa, running on an ESP8266.

A more detailed description can be found on [hackster.io](https://www.hackster.io/har-bra/irbridge-controlling-ir-devices-via-alexa-and-webinterface-66ca06).

## Folder contents
* C++ code that is running on the ESP8266, based on the Arduino core (in /esp8266)
* HTML/CSS/JS for webinterface, based on vue.js and Bulma (in /web)

## Licenses
* All code is licensed with the [MIT license](https://opensource.org/licenses/MIT)

## Development workflow & some hints
All code was developed in PlatformIO. After pulling the repository from GitHub, the following steps are required to be able to start developing:
* Library dependencies for the Arduino-portion of the code should be pulled automatically by PlatformIO based on `/esp8266/platformio.ini`.
* Adjust `lib_dir` and `lib_extra_dirs` in `/esp8266/platformio.ini` according to your local file setup.
* Library and development dependencies for the webinterface need to be installed with `npm install` (this requires Node.js on your development machine). This pulls all required dependencies as specified in `/web/package.json`.
* If changes on the webinterface have been done, use `npm run build` to package the files. The output is generated in `/web/dist/`. Copy the contents of this folder to `/esp8266/data/html/dist`.
* To upload changes in the file contents in `/esp8266/data` to the ESP8266's SPIFFS, use the PlatformIO "other target" `PIO Upload SPIFFS image`
* To upload changes of the general ESP8266 code, use the standard build target `PIO build` (doesn't overwrite SPIFFS file system)
* To save time during development and avoid repeated uploads to the SPIFFS memory (because they take time), connect the ESP8266 to the same Wifi network as your development machine and  uncomment the `baseURL` parameter in `/web/src/plugins/axios.js`. Set the `baseURL` to the ESP's IP address. Then use `npm run dev` to fire up a local development server. All API-calls to the ESP should still be answered by the ESP, however the webinterface will be served locally.
