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

#include <Arduino.h>
#include "IRBridge.h"
#include "lwip/init.h"

IRBridge irbridge;

#ifndef LWIP_HASH_STR
#define LWIP_HASH_STR "esp1.4"
#endif

void setup() {
    // this code is run once for setup

    // prepares the serial connection for logging purposes
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    /*// used to check the LWIP version that was used for compilation.
    // version prior to 2.0 result in continuous error messages
    printf("lwIP-%d.%d.%d(%s)",
            LWIP_VERSION_MAJOR,
            LWIP_VERSION_MINOR,
            LWIP_VERSION_REVISION,
            LWIP_HASH_STR);
    Serial.println("LWIP: ");
    Serial.println(LWIP_VERSION_MAJOR);*/

    // prepares the irbridge library
    irbridge.setup();
}

void loop() {
    // this code is run repeatedly

    // called to handle irbridge events
    irbridge.loop();
}
