# IRBridge - controlling IR devices via Alexa and webinterface

## Project description
The IRBridge can record and emit infrared commands - configured by a webinterface, accessible via Alexa, running on an ESP8266.

A more detailed description can be found on [hackster.io](https://www.hackster.io/har-bra/irbridge-controlling-ir-devices-via-alexa-and-webinterface-66ca06).

## Repository contents
* C++ code that is running on the ESP8266, based on the Arduino core (in /code/esp8266)
* HTML/CSS/JS for webinterface, based on vue.js and Bulma (in /code/web)
* Schematic and PCB for the underlying electronics, created in KiCAD (in /pcb)
* Enclosure design, created in FreeCAD (in /cad)

## Licenses
* All code is licensed with the [MIT license](https://opensource.org/licenses/MIT)
* Schematic, PCB and enclosure design are licensed with [CC-BY-SA-4.0](https://creativecommons.org/licenses/by-sa/4.0/)
