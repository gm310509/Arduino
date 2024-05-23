/**
  * gpschecksum.cpp
  * ---------------
  * 
  * Verify the checksum calculated and written to be a ublox GPS receiver.
  * Currently only recognises NMEA format GPS sentences.
  *
  * By: G. McCall
  *     May-2024
  *
  * Usage:
  *   gpschecksum file.Log
  *
  * History:
  *  23-May-2024
  *    Initial version.
  */

#define VERSION "1.00.00.00"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;


/* Convert a hexadecimal character to an integer. */
unsigned int hextoint(const char digit) {
  const char lookup [] = "0123456789ABCDEF";
  char ch = toupper(digit);
  int result = 0;

  for (result = 0; result < sizeof(lookup); result++) {
//    cout << "tohexint cmp: " << lookup[result] << " to " << ch << endl;
    if (lookup[result] == ch) {
      break;
    }
  }
//  cout << "hextoint (" << ch << ") " << result << endl;
  return result;
}


/*
 * Calculate and check the checksum in the GPS sentence
 * Return 0 if checksum is correct, or
 *        the checksum that the record *should* contain (but does not).
 */
int checksum(const char * sentence) {
  const char * p = sentence;


  unsigned int parity = 0;
  unsigned int chksum = 0;
  bool isCheckSumTerm = false;
  while (*p) {
    switch (*p) {
      case '$' :        // Start of sentence.
        parity = 0;
        isCheckSumTerm = false;
        break;
      case '*':         // End of data, start of checksum.
        isCheckSumTerm = true;
        chksum = 0;
        break;
      default:          // regular data character, so simply process it.
        if (!isCheckSumTerm) {
          parity ^= (unsigned int) *p;
        } else {
          chksum = (chksum << 4) | hextoint(*p);
        }
        break;
    }
    p++;
  }
//  cout << endl << uppercase << hex << "Parity: " << parity << " checksum: " << chksum << dec << endl;
 
  if (parity != chksum) {
    return parity;
  } else {
    return 0;
  }

}


/*
 * Process the data in the specified file.
 */
int process(const char * file) {
  cout << "Processing: " << file << endl;

  ifstream datafile(file);
  if (!datafile) {
    cout << "Error opening the file." << endl;
    return -1;
  }

  int lineCnt = 0;
  int errCnt = 0;
  string inLine;

  while (!datafile.eof()) {
    lineCnt++;
    getline(datafile, inLine);
    if (inLine.length() > 0) {
      int chksum = checksum(inLine.c_str());
      if (chksum != 0) {
        cout << lineCnt << ": " << inLine << endl;
        cout << "**** Invalid checksum. should be: 0x" << uppercase << hex << chksum << dec << endl;
        errCnt++;
      }
    }
  }
  datafile.close();


  cout << "processed: " << lineCnt << " lines. Errors: " << errCnt << endl;
  return lineCnt;
}


/* main
 * ----
 * Step through the command line arguments one by one.
 * Assume that the argument is a GPS log file and process it.
 */
int main(int argc, const char * argv[]) {
  
  for (int i = 1; i < argc; i++) {
    process(argv[i]);
  }
}

