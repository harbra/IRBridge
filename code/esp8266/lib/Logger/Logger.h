/* Logger.h

Purpose
=======

The Logger library is used as a central library to log events via a serial connection.

The goal behind having a separate library is in order to be able to completely disable logging
from a central location. This functionality is not imlemented yet.


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

#ifndef Logger_h
#define Logger_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

class Logger
{
public:
  typedef enum {
    DebugOutput,
    Info,
    Warning,
    Error
  } LoggerVerbosity;

  Logger();

  void addToLog(String message, LoggerVerbosity verbosity=LoggerVerbosity::DebugOutput);
  void addToLog(String message1, String message2, LoggerVerbosity verbosity=LoggerVerbosity::DebugOutput);

protected:


private:


};

extern Logger myLogger;

#endif
