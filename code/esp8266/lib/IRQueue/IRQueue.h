/* IRQueue.h

Purpose
=======

The IRQueue library is used to queue a number of events that are relevant for sending
IR commands with an Arduino-compatible device. This allows to have a "backlog" of
events to send out by the hardware once resources are available.

For sending IR events, the library [IRRemoteESP8266](https://github.com/markszabo/IRremoteESP8266) is used.


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

#ifndef IRQueue_h
#define IRQueue_h

#include <queue>
#include <vector>
#include <memory>

#include <Logger.h>

#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRsend.h>

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

// used for child classes of IRBase to make it transparent for the code which
// kind of IR object is used
typedef enum {
  Base,
  Protocol,
  Raw,
  Pause
} IRType;

// base class for IR objects
class IRBase
{
public:
  IRType type;
  IRBase() : type(IRType::Base), repeat(0) {};
  IRBase(IRType mytype) : type(mytype), repeat(0) {};
  IRBase(IRType mytype, uint16_t myrepeat) : type(mytype), repeat(myrepeat) {};

  uint16_t repeat;
};

// represents IR events that are identified by protocol information (i.e. manufacturer,
// value, etc.)
class IRProtocol : public IRBase
{
public:
  IRProtocol() : IRBase(IRType::Protocol), protocol(0), value(0), bits(0) {};
  IRProtocol(int myprotocol, uint64_t myvalue, uint16_t mybits, uint16_t myrepeat) : IRBase(IRType::Protocol, myrepeat), protocol(myprotocol), value(myvalue), bits(mybits) {};
  int protocol;
  uint64_t value;
  uint16_t bits;
};

// represents IR events that are defined by raw timing values
class IRRaw : public IRBase
{
public:
  IRRaw() : IRBase(IRType::Raw), freq(0) {};
  IRRaw(std::vector<uint16_t> myvalues, uint16_t myfreq, uint16_t myrepeat) : IRBase(IRType::Raw, myrepeat), freq(myfreq), values(myvalues) {};
  std::vector<uint16_t> values;
  uint16_t freq;
};

// represents a pause between firing consecutive IR events
class IRPause : public IRBase
{
public:
  IRPause() : IRBase(IRType::Pause), duration(0) {};
  IRPause(uint32_t myduration) : IRBase(IRType::Pause, 0), duration(myduration) {};
  uint32_t duration;
};

// main class
class IRQueue
{
public:

  // should be initialized with the output pin that the IR led is connected to
  IRQueue(int irpin);

  // should be called in loop to handle events in the queue
  void process();

  // add a new IR event to the queue
  void add(std::shared_ptr<IRBase> obj) { m_queue.push(obj); };

  // reset the IR queue and delete all events that have not been fired yet
  void clear() { while(!m_queue.empty()) { m_queue.pop(); }};

protected:


private:
  std::queue<std::shared_ptr<IRBase>> m_queue;

  // IR related stuff
  IRsend irsend;

};

#endif
