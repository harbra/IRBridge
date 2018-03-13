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

#include "IRQueue.h"

IRQueue::IRQueue(int irpin) : irsend(irpin)
{
  irsend.begin();
}

void IRQueue::process()
{
  // don't to anything if the queue is empty
  while(!m_queue.empty()) {

    // determine what kind of IRBase object is next in the queue and handle it accordingly
    if(m_queue.front()->type==IRType::Protocol)
    {
      // first cast the IRBase object to the correct child class
      std::shared_ptr<IRProtocol> protocol=std::static_pointer_cast<IRProtocol>(m_queue.front());

      int ir_type=protocol->protocol;
      uint64_t code=protocol->value;
      uint16_t bits=protocol->bits;
      uint16_t repeat=protocol->repeat;

      myLogger.addToLog("Sending protocol: ",String(ir_type));
      myLogger.addToLog("code: ",String((unsigned long)((code & 0xFFFFFFFF00000000) >> 32 ),HEX));
      myLogger.addToLog("code: ",String((unsigned long)((code & 0x00000000FFFFFFFF)),HEX));
      myLogger.addToLog("bits: ",String(bits));
      myLogger.addToLog("repeat: ",String(repeat));

      // this code is mainly borrowed from IRRemoteESP8266 examples
      switch (ir_type) {
      #if SEND_RC5
          case RC5:  // 1
            if (bits == 0)
              bits = RC5_BITS;
            irsend.sendRC5(code, bits, repeat);
            break;
      #endif
      #if SEND_RC6
          case RC6:  // 2
            if (bits == 0)
              bits = RC6_MODE0_BITS;
            irsend.sendRC6(code, bits, repeat);
            break;
      #endif
      #if SEND_NEC
          case NEC:  // 3
            if (bits == 0)
              bits = NEC_BITS;
            irsend.sendNEC(code, bits, repeat);
            break;
      #endif
      #if SEND_SONY
          case SONY:  // 4
            if (bits == 0)
              bits = SONY_12_BITS;
            repeat = std::max(repeat, (uint16_t) SONY_MIN_REPEAT);
            irsend.sendSony(code, bits, repeat);
            break;
      #endif
      #if SEND_PANASONIC
          case PANASONIC:  // 5
            if (bits == 0)
              bits = PANASONIC_BITS;
            irsend.sendPanasonic64(code, bits, repeat);
            break;
      #endif
      #if SEND_JVC
          case JVC:  // 6
            if (bits == 0)
              bits = JVC_BITS;
            irsend.sendJVC(code, bits, repeat);
            break;
      #endif
      #if SEND_SAMSUNG
          case SAMSUNG:  // 7
            if (bits == 0)
              bits = SAMSUNG_BITS;
            irsend.sendSAMSUNG(code, bits, repeat);
            break;
      #endif
      #if SEND_WHYNTER
          case WHYNTER:  // 8
            if (bits == 0)
              bits = WHYNTER_BITS;
            irsend.sendWhynter(code, bits, repeat);
            break;
      #endif
      #if SEND_AIWA_RC_T501
          case AIWA_RC_T501:  // 9
            if (bits == 0)
              bits = AIWA_RC_T501_BITS;
            repeat = std::max(repeat, (uint16_t) AIWA_RC_T501_MIN_REPEAT);
            irsend.sendAiwaRCT501(code, bits, repeat);
            break;
      #endif
      #if SEND_LG
          case LG:  // 10
            if (bits == 0)
              bits = LG_BITS;
            irsend.sendLG(code, bits, repeat);
            break;
      #endif
      #if SEND_MITSUBISHI
          case MITSUBISHI:  // 12
            if (bits == 0)
              bits = MITSUBISHI_BITS;
            repeat = std::max(repeat, (uint16_t) MITSUBISHI_MIN_REPEAT);
            irsend.sendMitsubishi(code, bits, repeat);
            break;
      #endif
      #if SEND_DISH
          case DISH:  // 13
            if (bits == 0)
              bits = DISH_BITS;
            repeat = std::max(repeat, (uint16_t) DISH_MIN_REPEAT);
            irsend.sendDISH(code, bits, repeat);
            break;
      #endif
      #if SEND_SHARP
          case SHARP:  // 14
            if (bits == 0)
              bits = SHARP_BITS;
            irsend.sendSharpRaw(code, bits, repeat);
            break;
      #endif
      #if SEND_COOLIX
          case COOLIX:  // 15
            if (bits == 0)
              bits = COOLIX_BITS;
            irsend.sendCOOLIX(code, bits, repeat);
            break;
      #endif
      #if SEND_DENON
          case DENON:  // 17
            if (bits == 0)
              bits = DENON_BITS;
            irsend.sendDenon(code, bits, repeat);
            break;
      #endif
      #if SEND_SHERWOOD
          case SHERWOOD:  // 19
            if (bits == 0)
              bits = SHERWOOD_BITS;
            repeat = std::max(repeat, (uint16_t) SHERWOOD_MIN_REPEAT);
            irsend.sendSherwood(code, bits, repeat);
            break;
      #endif
      #if SEND_RCMM
          case RCMM:  // 21
            if (bits == 0)
              bits == RCMM_BITS;
            irsend.sendRCMM(code, bits, repeat);
            break;
      #endif
      #if SEND_SANYO
          case SANYO_LC7461:  // 22
            if (bits == 0)
              bits = SANYO_LC7461_BITS;
            irsend.sendSanyoLC7461(code, bits, repeat);
            break;
      #endif
      #if SEND_RC5
          case RC5X:  // 23
            if (bits == 0)
              bits = RC5X_BITS;
            irsend.sendRC5(code, bits, repeat);
            break;
      #endif
      #if SEND_PRONTO
          case PRONTO:  // 25
            // TODO: PRONTO
            break;
      #endif
      #if SEND_NIKAI
          case NIKAI:  // 29
            if (bits == 0)
              bits = NIKAI_BITS;
            irsend.sendNikai(code, bits, repeat);
            break;
      #endif
      #if SEND_GLOBALCACHE
          case GLOBALCACHE:  // 31
            // TODO: GLOBALCACHE
            break;
      #endif
      #if SEND_MIDEA
          case MIDEA:  // 34
            if (bits == 0)
              bits = MIDEA_BITS;
            irsend.sendMidea(code, bits, repeat);
            break;
      #endif
      #if SEND_MAGIQUEST
          case MAGIQUEST:  // 35
            if (bits == 0)
              bits = MAGIQUEST_BITS;
            irsend.sendMagiQuest(code, bits, repeat);
            break;
      #endif
      #if SEND_LASERTAG
          case LASERTAG:  // 36
            if (bits == 0)
              bits = LASERTAG_BITS;
            irsend.sendLasertag(code, bits, repeat);
            break;
      #endif
      #if SEND_CARRIER_AC
          case CARRIER_AC:  // 37
            if (bits == 0)
              bits = CARRIER_AC_BITS;
            irsend.sendCarrierAC(code, bits, repeat);
            break;
      #endif
      }
    } else if(m_queue.front()->type==IRType::Raw)
    {
      // first cast the IRBase object to the correct child class
      std::shared_ptr<IRRaw> raw=std::static_pointer_cast<IRRaw>(m_queue.front());
      myLogger.addToLog("sending raw...");

      for(int i=0; i<raw->values.size(); i++)
      {
        myLogger.addToLog(String("raw[")+i+"]: "+raw->values[i]);
      }
      for(int i=0; i<=raw->repeat; i++)
      {
        irsend.sendRaw(raw->values.data(), raw->values.size(), raw->freq);
        delay(250);
      }
    } else if(m_queue.front()->type==IRType::Pause)
    {
      // first cast the IRBase object to the correct child class
      std::shared_ptr<IRPause> pause=std::static_pointer_cast<IRPause>(m_queue.front());
      myLogger.addToLog("sending pause...");
      delay(pause->duration*1000);
    }

    m_queue.pop();
  }
}
