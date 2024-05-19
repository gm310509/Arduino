#include "Utility.h"


/**
 * rightJustify
 * 
 * Right justify a long integer into a buffer that is size fieldSize + 1. 
 * The filler character is used to pad the number to the right.
 * 
 * @params
 *   buf - a buffer that is big enough to hold the converted number *and* a trailing null byte.
 *   fieldSize - the number of characters into which the number is formatted.
 *   x - the number to convert.
 *   filler - the character used to pad out the converted number.
 * 
 * @return
 *   a pointer to the buffer.
 * 
 * @note
 * The buffer must be at least fieldSize + 1 characters in size.
 * 
 * If the fieldSize is too small for the converted integer, then the buffer will not contain
 * all of the digits representing the number.
 * 
 */
char * rightJustify(char *buf, int fieldSize, long x, char filler, char seperator) {
  bool negativeSign = false;

  // Check for a -ve sign and remember this for later.
  // ensure we have |x| for conversion.
  if ((negativeSign = x < 0)) {
    x = -x;
  }

  // NULL terminate the string.
  char *p = buf + fieldSize;
  *p-- = '\0';

  // Load up the characters representing the number.
  int digitCount = 0;
  do {
    *p-- = x % 10 + '0';
    x /= 10;
    digitCount++;
    if (seperator && digitCount % 3 == 0 && x && p >= buf) {
      *p-- = seperator;
    }
  } while (x && p >= buf);

  // Do we need a negative sign? If so, put it in.
  if (p >= buf && negativeSign) {
    *p-- = '-';
  }

  // Pad out the string.
  while (p >= buf) {
    *p-- = filler;
  }
  return buf;
}



char * rightJustifyF(char *buf, int fieldSize, double x, int dpCnt, char filler, char seperator) {

  long iPart = (long) x;

  double fPart = x - (double) iPart;

  rightJustify(buf, fieldSize - 1 - dpCnt, iPart, filler, seperator);
  char *p = buf + fieldSize - 1 - dpCnt;
  if (p < buf + fieldSize) {
    *p++ = '.';
  }

  // When outputing the fractional component, we need our number to be positive.
  if (fPart < 0) {
    fPart = -fPart;
  }

  while (p < buf + fieldSize && dpCnt--) {
    double tmp = fPart * 10.0;
    int digit = (int)tmp;
    *p++ = digit + '0';
    fPart = tmp - digit;
  }

  buf[fieldSize] = '\0';
  return buf;
}



char * rightJustifyF(char *buf, int fieldSize, double x, char filler, char seperator) {
  return rightJustifyF(buf, fieldSize, x, 2, filler, seperator);
}

