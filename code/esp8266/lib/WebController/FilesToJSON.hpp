/* FilesToJSON.hpp

Purpose
=======

FilesToJSON.hpp is used as container to send files located in the SPIFSS memory
to a connected client in JSON format and works together with the ESPAsyncWebServer library.
Which files are included can be limited by white- and blacklisting.

The file contents are base64 encoded.


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

#ifndef FilesToJSON_h
#define FilesToJSON_h

#include <FS.h>
#include <vector>
#include "base64.h"

#include <Logger.h>

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

class FilesToJSON
{
public:

  // constructor that accepts a list of includepaths (= whitelist) and
  // blacklistfiles (= blacklist)
  // whitelist should contain paths (e.g. /myfolder/)
  // blacklist should contain filenames (e.g. /myfolder/dontsend.raw)
  // if base64encode is true, the files are added base64 encoded as JSON objects
  FilesToJSON(std::vector<String> includepaths,std::vector<String> blacklistfiles, bool base64encode=true) : done(false), myIncludePaths(includepaths), myBlacklistFiles(blacklistfiles), mybase64encode(base64encode) {
    myDir=SPIFFS.openDir("/");
  }

  // callback function used by ESPAsyncWebServer to get next chunk of data
  // see ESPAsyncWebServer documentation for more details
  size_t continueFiles(uint8_t* buffer, size_t maxlen) {
    if(done) {
      // when everything has been sent, function can return here
      return 0;
    }

    size_t bytesWritten=0;

    // On first call, fileName is still empty. If this is true, initialize the buffer
    // as required for a valid JSON array.
    if(myDir.fileName()=="") {
      myLogger.addToLog("New JSON");
      buffer[0]='[';
      buffer++;
      bytesWritten++;
    }

    bool filesDone=false;

    // Loop as long as there is still free space in the buffer
    while(true) {
      if(bytesWritten>=maxlen) return maxlen;

      if(!myFile.available()) {
        // file contents have already been sent completely.
        // look for next or terminate

        if(!myDir.next()) {
          // no more files found. end the JSON file and terminate loop
          filesDone=true;
          myLogger.addToLog("Done with files");
          if(stringBuffer=="") {
            if(mybase64encode) {
              stringBuffer="\"}]";
            } else {
              stringBuffer="]";
            }
          }
          break;
        }

        // another file has been found
        // check if file is in includedirs and not in blacklistpaths
        bool skip=true;
        for(auto path : myIncludePaths) {
          if(myDir.fileName().startsWith(path)) {
            skip=false;
          }
        }

        for(auto filename : myBlacklistFiles) {
          if(myDir.fileName().equalsIgnoreCase(filename)) {
            skip=true;
          }
        }

        // go to next file if file is not in include paths or blacklisted
        // myFile.available while still be false, so next file will be queried
        if(skip) {
          continue;
        }

        // continue handling file
        // file has been sent completely already, thus terminate the current JSON object
        // and open next file
        stringBuffer="";
        if(myFile) {
          myLogger.addToLog("File done");
          if(mybase64encode) {
            stringBuffer+="\"},";
          } else {
            stringBuffer+=",";
          }
        }
        myFile.close();
        myLogger.addToLog("Starting next file");
        if(mybase64encode) {
          stringBuffer+="{\"filepath\":\""+myDir.fileName()+"\"";
          stringBuffer+=",\"content\":\"";
        }
        myFile=myDir.openFile("r");
      }

      if(stringBuffer!="") {
        // there is content left in the stringBuffer, write as much of that content to the buffer
        // as possible
        myLogger.addToLog("Writing String: ",String(stringBuffer.length()));
        size_t nextstringlength=std::min(maxlen-bytesWritten,stringBuffer.length());
        myLogger.addToLog("nextstringlength: ",String(nextstringlength));

        strncpy((char *) buffer, stringBuffer.c_str(), nextstringlength);
        bytesWritten+=nextstringlength;
        buffer+=nextstringlength;
        stringBuffer=stringBuffer.substring(nextstringlength);
      }

      if(stringBuffer=="") {
        // the stringBuffer is empty, thus the file contents can now be written

        if(mybase64encode) {
          // determine how much of the file contents will fit into the buffer
          // this considers that base64 encoding will increase the content size and
          // happens in chungs of 3 bytes
          size_t base64length=((myFile.size()-myFile.position())*1.4f)+1;
          size_t nextavailablelength=std::min(maxlen-bytesWritten,base64length);
          size_t nextreadlength;
          if(nextavailablelength==base64length) {
            nextreadlength=myFile.size()-myFile.position();
          } else {
            nextreadlength=((nextavailablelength-1)*1.0f)/1.4f;
            nextreadlength-=(nextreadlength%3);
          }

          if(nextreadlength<3) return bytesWritten;

          uint8_t* tempBuffer=new uint8_t[nextreadlength];
          size_t tempBytesWritten=myFile.read(tempBuffer,nextreadlength);
          String encoded=base64::encode(tempBuffer,tempBytesWritten,false);
          delete tempBuffer;
          strncpy((char *) buffer, encoded.c_str(), encoded.length());
          bytesWritten+=size_t(encoded.length());
          buffer+=size_t(encoded.length());
        } else {
          size_t nextreadlength=std::min(maxlen-bytesWritten,myFile.size()-myFile.position());
          bytesWritten+=myFile.read(buffer,nextreadlength);
          buffer+=nextreadlength;
        }
      }
    }

    if(filesDone) {
      // all files have been sent, close JSON string
      if(stringBuffer!="") {
        myLogger.addToLog("Writing String");
        size_t nextstringlength=std::min(maxlen-bytesWritten,stringBuffer.length());

        strncpy((char *) buffer, stringBuffer.c_str(), nextstringlength);
        bytesWritten+=nextstringlength;
        buffer+=nextstringlength;
        stringBuffer=stringBuffer.substring(nextstringlength);
      }

      if(stringBuffer=="") {
        myLogger.addToLog("done");
        done=true;
        return bytesWritten;
      } else {
        return bytesWritten;
      }
    }
  }

private:

  Dir myDir;
  File myFile;

  std::vector<String> myIncludePaths;
  std::vector<String> myBlacklistFiles;
  String stringBuffer;
  bool done;
  bool mybase64encode;

};

#endif
