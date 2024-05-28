/*****************
 * Program to print and check a hab log.
 *
 * By: G. McCall
 *     May 2024
 *
 *  v1.01.00.00 28-05-2024
 *    * Added ability to submit a sentence for checksum verification
 *      via the Console.
 *  V1.00.00.00 23-05-2024
 *    * Initial version.
 *  
 */


#define VERSION "v1.01.00.00"

// Baud rate of the Serial (PC USB connection) device.
#define CONSOLE_BAUD 115200

#include "Buffer.h"
#include "utility.h"


#include <SdFat.h>
#include <sdios.h>


Buffer consoleBuf;
bool echoConsoleInput = true;


#define MAX_LOG_FILE_NAME_SIZE 12
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3


// For Teensy 3.5 & 3.6 & 4.1 on-board: BUILTIN_SDCARD
#define SD_CD_PIN = BUILTIN_SDCARD;

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
File root;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
File32 root;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
ExFile root;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
FsFile root;
#endif  // SD_FAT_TYPE


// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS


char currDir[200] = "/";


void listFiles(const char * dir) {
  char wrkBuf[80];

  int fileCnt = 0;
  if (root.open(dir)) {
    while (file.openNext(&root, O_RDONLY)) {
      if (file.isFile()) {
        file.getName(wrkBuf, sizeof(wrkBuf));
        Serial.print(wrkBuf);
        Serial.print(": (");
        Serial.print(file.size());
        Serial.println(" bytes)");
        fileCnt++;
      }
      file.close();
    }
    Serial.print(fileCnt); Serial.println(" files.");
  }
}


void pwd(const char * cmd, char const *tokens[], int tokenCnt) {
  Serial.println(currDir);
}


void cd(const char * cmd, const char *tokens[], int tokenCnt) {
  if (tokenCnt == 2) {
    strcpy(currDir, tokens[1]);
  }
}


void ls(const char * cmd, const char *tokens[], int tokenCnt) {
  pwd(cmd, tokens, tokenCnt);
  listFiles(currDir);
}

void outputFile(const char * fileName, unsigned long headSize) {

  if (!file.open(fileName, FILE_READ)) {
    Serial.print("Failed to open: "); Serial.println(fileName);
    return;
  }

  char inLine[200];
  unsigned long lineNo = 0;

  while (file.available() && ((headSize > 0 && lineNo < headSize) || headSize == 0)) {
    int n = file.fgets(inLine, sizeof(inLine));
    if (n <= 0) {
      Serial.println("Error reading file");
    }
    lineNo++;
    if (inLine[n - 1] != '\n' && n == (sizeof(inLine) - 1)) {
      Serial.println("Line too long:");
      Serial.println(inLine);
    } else {
      Serial.print(inLine);
    }
  }

  file.close();
}


void headFile(const char * cmd, char const *tokens[], int tokenCnt) {
  const char * fileName = tokens[1];
  long headSize = 20;
  if (tokenCnt == 2) {
    fileName = tokens[1];
  } else if (tokenCnt == 3) {
    if (isNumeric(tokens[1])) {
      headSize = atol(tokens[1]);
    }
    fileName = tokens[2];
  } else {
    Serial.println("Error, specify one file only with an option head size.");
    return;
  }
  outputFile(fileName, headSize);
}



void catFile(const char * cmd, char const *tokens[], int tokenCnt) {
  const char * fileName = tokens[1];
  if (tokenCnt != 2) {
    Serial.println("Error, specify one file only with an option head size.");
    return;
  }
  outputFile(fileName, 0);
}





/* Convert a hexadecimal character to an integer. */
unsigned int hextoint(const char digit) {
  const char lookup [] = "0123456789ABCDEF";
  char ch = toupper(digit);
  unsigned int result = 0;

  for (result = 0; result < sizeof(lookup); result++) {
    if (lookup[result] == ch) {
      break;
    }
  }
  return result;
}


/*
 * Calculate and check the checksum in the GPS sentence
 * Return 0 if checksum is correct, or
 *        the checksum that the record *should* contain (but does not).
 */
unsigned int chkSentence(const char * sentence) {
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
      case '\n':
      case '\r':
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
  } else if (!isCheckSumTerm) {
    return -1;
  } else {
    return 0;
  }
}



void chkFile(const char * cmd, char const *tokens[], int tokenCnt) {
  const char * fileName = tokens[1];
  unsigned long headSize = 20;
  if (tokenCnt == 2) {
    fileName = tokens[1];
  } else if (tokenCnt == 3) {
    if (isNumeric(tokens[1])) {
      headSize = atol(tokens[1]);
    }
    fileName = tokens[2];
  } else {
    Serial.println("Error, specify one file only with an option head size.");
    return;
  }


  char inLine[200];
  unsigned long lineNo = 0;
  unsigned long errCnt = 0;

  if (!file.open(fileName, FILE_READ)) {
    Serial.print("Failed to open: "); Serial.println(fileName);
    return;
  }

  while (file.available()) {
    int n = file.fgets(inLine, sizeof(inLine));
    if (n <= 0) {
      Serial.println("Error reading file");
      continue;
    }
    if (lineNo++ < headSize) {  // Skip the first n lines.
      continue;
    }
    if (strncmp("$HAB", inLine, 4) == 0) {
      continue;         // Skip the $HAB records.
    }
    if (inLine[n - 1] != '\n' && n == (sizeof(inLine) - 1)) {
      Serial.print("line "); Serial.print(lineNo); Serial.println(" too long:");
      Serial.println(inLine);
    }

    unsigned int checksum = chkSentence(inLine);
    if (checksum != 0) {
      Serial.print(lineNo); Serial.print(": ");
      Serial.print(inLine);
      Serial.print("*** invalid checskum. Should be: 0x"); Serial.println(checksum, HEX);
      Serial.println();
      errCnt++;
    }
  }

  file.close();

  Serial.print("Lines checked: "); Serial.println(lineNo);
  Serial.print("Checksum errors: "); Serial.println(errCnt);
  if (lineNo > 0) {
    Serial.print("Percentage error: "); Serial.print((double) errCnt / (double)(lineNo) * 100.0); Serial.println("%");
  }
}


void validateSentence(const char * sentence) {
    Serial.println(sentence);
    unsigned int checksum = chkSentence(sentence);
    if (checksum == 0xffffffff) {
      Serial.println(F("*** GPS sentence does not seem to include a checksum sequence"));
    } else if (checksum != 0) {
      Serial.print(F("*** invalid checskum. Should be: 0x")); Serial.println(checksum, HEX);
      Serial.println();
    } else {
      Serial.println(F("Checksum OK"));
    }
}


/************************************
 * Command Processing
 ************************************/

void usage() {
  Serial.println(F("Usage:"));
  Serial.println(F("  ls              list files in current directory."));
  Serial.println(F("  cd dir          Change current directory to dir."));
  Serial.println(F("  pwd             Print current directory."));
  Serial.println(F("  head [n] file   Print first n lines of the file (n = 20)."));
  Serial.println(F("  cat file        Print entire contents of the file."));

  Serial.println();

  Serial.println(F("  chk [n] file    Check the GPS checksums for validity."));
  Serial.println(F("                  Skip the first n lines (n = 5)"));

  Serial.println(F("  $sentence*chk   Verify the checksum of the provided GPS sentence."));

  Serial.println();
  Serial.println(F("  echo on|off     set command echo."));
  Serial.println(F("  help|usage      show commands."));
}



/* Invalid command observed.
 */
void invalid(const char * cmd) {
  Serial.print(F("Invalid Command: "));
  Serial.println(cmd);
  usage();
}


/*
 * process the echo command.
 * command: 
 *   echo on|off          -> echoConsolInput
 */
void processEcho(const char * cmd, char const *tokens[], int tokenCnt) {
  if (tokenCnt == 2) {          // Set echo state.
    if (strcmp(tokens[1], "on") == 0) {
      echoConsoleInput = true;
    } else if (strcmp(tokens[1], "off") == 0) {
      echoConsoleInput = false;
    } else {
      Serial.println("Usage: echo [on|off]");
      invalid(cmd);
    }
  } else if (tokenCnt == 1) {    // Report echo state.
    Serial.print(F("Echo: "));
    Serial.println(echoConsoleInput ? F("on") : F("off"));
  } else {                      // Invalid command.
    invalid(cmd);
  }
}




/************************************
 * Console Interactions
 ************************************/
/**
 * Output a prompt if we are using a terminal (characer echo is turned on).
 * If echo is not turned on, assume a "local echo" terminal such as the Arduino Serial monitor.
 */
void prompt() {
  if (echoConsoleInput) {
    Serial.print(F("--> "));
  }
}




/*************************************
 * Console/Command processing functions.
 * 
 *************************************/
/**
 * ProcessConsoleMessage
 * ---------------------
 * 
 * Processes a message received from the console device.
 * 
 * @param msg the message received from the remote.
 */
#define MAX_TOKENS 10
void processConsoleCommand(const char *cmd) {

  if (strlen(cmd) == 0) {
    return;
  }

  // Convert input to lower case.
  char wrk[200];
  // char *s = cmd;
  // char *t = wrk;
  // while (*s) {
  //   *t++ = tolower(*s++);
  // }
  // *t = '\0';
  strcpy (wrk, cmd);

  if (wrk[0] == '$') {
    validateSentence(cmd);
    return;
  }

  // Tokenise the input
  char *tokens[MAX_TOKENS] = {0};
  const char * separators = " ";
  char *tok = strtok(wrk, separators);

  int tokenCount = 0;
  while (tok != 0) {
    if (tokenCount < MAX_TOKENS) {
      tokens[tokenCount++] = tok;
    } else {
      Serial.print(F("Too many tokens input. Max="));
      Serial.println(MAX_TOKENS);
    }
    tok = strtok(0, separators);
  }

//  for (int i = 0; i < tokenCount; i++) {
//    if (i < 10) {
//      Serial.print(" ");
//    }
//    Serial.print(i);
//    Serial.print(": ");
//    Serial.println(tokens[i]);
//  }

  // Determine the command and call its handler.
  if (stricmp(tokens[0], "echo") == 0) {
    processEcho(cmd, tokens, tokenCount);
  } else if (stricmp(tokens[0], "pwd") == 0) {
    pwd(cmd, tokens, tokenCount);
  } else if (stricmp(tokens[0], "cd") == 0) {
    cd(cmd, tokens, tokenCount);
  } else if (stricmp(tokens[0], "ls") == 0) {
    ls(cmd, tokens, tokenCount);
  } else if (stricmp(tokens[0], "head") == 0) {
    headFile(cmd, tokens, tokenCount);
  } else if (stricmp(tokens[0], "cat") == 0) {
    catFile(cmd, tokens, tokenCount);
  } else if (stricmp(tokens[0], "chk") == 0) {
    chkFile(cmd, tokens, tokenCount);
  } else if (stricmp(tokens[0], "help") == 0 || strcmp(tokens[0], "usage") == 0) {
    usage();
  } else {
    invalid(cmd);
  }
}




/*
 * Check for console input.
 * Accumulate any input into a buffer.
 * Process it upon seeing a line terminator.
 */
void checkConsoleInput() {
  if (Serial.available() > 0) {
    char ch = Serial.read();
    if (echoConsoleInput) {
      Serial.print(ch);
    }

    if (ch == '\n' || ch == '\r') { // Do we have a line terminator (LF || CR) which marks the end of the input.
      Serial.println();
      processConsoleCommand(consoleBuf.getMsg());   // Process the input
      consoleBuf.reset();           // Reset the buffer Pointer for the next input.
    } else if (ch == '\b') {        // Just in case we are using a terminal,
      consoleBuf.remove();          // remove 1 character.
      if (echoConsoleInput) {
        Serial.print(F(" \b"));    // erase the character from the terminal. We already echoed the BS, so replace the character with a space and BS again.
      }
    } else {
      // Not a CR and not a LF, so just accumulate the character.
      consoleBuf.append(ch);
    }
  }
}



void setup() {
  Serial.begin(CONSOLE_BAUD);

  if (!sd.begin(SD_CONFIG)) {
    Serial.println("Error initialising the sd card module.");
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) {
      digitalWrite(LED_BUILTIN, ! digitalRead(LED_BUILTIN));
      delay(150);
    }
  }
  Serial.println("SD Card monitor");
  Serial.println(F("Enter \"help\" for list of commands."));
}



void loop() {
  checkConsoleInput();

}
