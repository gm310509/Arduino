#include <Arduino.h>
#include "Buffer.h"


// Macro to access text of a #define in code.
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

/* Example usage:
 *  
 *  #define CONST Serial3
 *  
 *  Serial.println(STR(CONST));
 *  will cause the text "Serial3" to be printed.
 */


boolean Buffer::append(char ch) {
  if (bufPtr < BUFFER_SIZE - 1) {
    buf[bufPtr++] = ch;         // Append the character to the buffer (if space is available).
    buf[bufPtr] = '\0';       // Null terminate the string.
    return true;              // success.
  } else {
//    Serial.println("Overflow");
    return false;             // failure - buffer overflow.
  }
}

    
boolean Buffer::remove(const unsigned int cnt) {
  if (bufPtr >= cnt) {
    bufPtr -= cnt;
    buf[bufPtr] = '\0';
    return true;
  } else {
//    Serial.println("Underflow");
    return false;
  }
}


void Buffer::clear() {
  bufPtr = 0;
  buf[bufPtr] = '\0';
}


int Buffer::getLen() {
  return bufPtr;
}

const char * Buffer::getMsg() {
  return &buf[0];
}
