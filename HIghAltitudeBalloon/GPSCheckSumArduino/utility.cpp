#include <Print.h>

#include "utility.h"

/* Checks whether a given string is numeric or not
 * return 0 for no, 1 for yes.
 * Permist floating point and exponential numbers.
 */
int isNumeric(const char * str)
{
    int stringLength = strlen(str);
    int i = 0, j = stringLength - 1;
    bool flagDotOrE = false;
 
    // Handling whitespaces
    while (i < stringLength && str[i] == ' ')
        i++;
    while (j >= 0 && str[j] == ' ')
        j--;
 
    if (i > j)
        return 0;
 
    // if string is of length 1 and the only
    // character is not a digit
    if (i == j && !(str[i] >= '0' && str[i] <= '9'))
        return 0;
 
    // If the 1st char is not '+', '-', '.' or digit
    if (str[i] != '.' && str[i] != '+'
        && str[i] != '-' && !(str[i] >= '0' && str[i] <= '9'))
        return 0;
 
    // To check if a '.' or 'e' is found in given
    // string. We use this flag to make sure that
    // either of them appear only once.
 
    for (; i <= j; i++) {
        // If any of the char does not belong to
        // {digit, +, -, ., e}
        if (str[i] != 'e' && str[i] != '.'
            && str[i] != '+' && str[i] != '-'
            && !(str[i] >= '0' && str[i] <= '9'))
            return 0;
 
        if (str[i] == '.') {
            // checks if the char 'e' has already
            // occurred before '.' If yes, return 0.
            if (flagDotOrE == true)
                return 0;
 
            // If '.' is the last character.
            if (i + 1 > stringLength)
                return 0;
 
            // if '.' is not followed by a digit.
            if (!(str[i + 1] >= '0' && str[i + 1] <= '9'))
                return 0;
        }
 
        else if (str[i] == 'e') {
            // set flagDotOrE = 1 when e is encountered.
            flagDotOrE = true;
 
            // if there is no digit before 'e'.
            if (!(str[i - 1] >= '0' && str[i - 1] <= '9'))
                return 0;
 
            // If 'e' is the last Character
            if (i + 1 > stringLength)
                return 0;
 
            // if e is not followed either by
            // '+', '-' or a digit
            if (str[i + 1] != '+' && str[i + 1] != '-'
                && (str[i + 1] >= '0' && str[i] <= '9'))
                return 0;
        }
    }
 
    /* If the string skips all above cases, then 
    it is numeric*/
    return 1;
}




/*
 * Format a number to two digits with leadning zero.
 * Only works for numbers 0-99.
 */
char * twoDigits(unsigned int num, char * buf) {
  if (num < 10) {
    buf[0] = '0';
  } else {
    buf[0] = num / 10 + '0';
  }
  buf [1] = num % 10 + '0';
  buf[2] = '\0';
  return buf;
}





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


