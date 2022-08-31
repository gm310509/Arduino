/**
 * SubredditStats
 * --------------
 * 
 * A basic class that captures and tracks various subreddit
 * metrics.
 * 
 * This class also has some placeholder functions for attaching
 * a display device of some sort. These are all "do nothing"
 * functions that need to be implemented to suit the particular type
 * of display.
 */
#include <Arduino.h>
#include "SubredditStats.h"

/**
 * Define some standard text labels for the
 * metrics captured.
 */
const char *labels[] = {
  "Active: ",     // 0
  "Subs: ",       // 1
  "New today: ",  // 2
  " 7 days: ",    // 3
  "30 days: ",    // 4
  "90 days: ",    // 5
  "Tgt: ",        // 6
  "Est: ",        // 7
  ""              // 8 - Name of the subreddit
};

/**
 * Accept a new (comma seperated) list of values.
 * Seperate the values and remember them.
 */
void SubredditStats::registerInputData(char *statsText) {
  if (strlen(statsText) == 0) {       // Empty input?
    return;                           // Yes, nothing to do here, so just return.
  }

        // Echo what we have received.
  Serial.print("Got: '"); Serial.print(statsText); Serial.println("'");
        // Save the data received so that it doesn't get
        // overwritten as new data is sent to us.
  strcpy(statsTextBuf, statsText);
  Serial.print("Length: "); Serial.println(strlen(statsText));

  // Process the input that we have received.
  // Identify each individual string as seperated by commas.
  int cnt = 0;
  const char delim[] = ",";
  char *token = strtok(statsTextBuf, delim);
  while (token != NULL) {
    if (cnt < MAX_MSG_CNT) {      // Ensure we do not overflow the number of tokens we track.
      values[cnt] = token;        // Remember the first MAX_MSG_CNT items.
    } else {
      Serial.print("MAX_MSG_CNT overflow, cnt = "); Serial.print(cnt); Serial.print(", ");
    }
    Serial.println(token);        // Debug: print the token extracted.
    token = strtok(NULL, delim);  // Get the next token.
    cnt++;                        // Point to the next token to be remembered.
  }
  Serial.print(cnt); Serial.println(" tokens found");
  _hasValidData = true;           // Flag that we have received some data (which hopefully is valid).
}


/**
 * Retrieve the value of a metric as indexed by the itemNo.
 * The metric is stored as a string, so this method returns a pointer to a char array.
 */
const char *SubredditStats::getValue(int itemNo) {
  if (itemNo >= 0 && itemNo < MAX_MSG_CNT) {
    return values[itemNo];
  } else {
    return "";
  }
}


/**
 * Retrieve the standard label corresponding to a metric as indexed by the itemNo.
 */
const char *SubredditStats::getLabel(int itemNo) {
  if (itemNo >= 0 && itemNo < MAX_MSG_CNT) {
    return labels[itemNo];
  } else {
    return "??";
  }
}

/**
 * A place holder that should be called when a display needs to be initialised.
 * This function should be overriden by a display module (if needed).
 */
void SubredditStats::initDisplay() {
}


/**
 * A place holder that is called when a message is to be output.
 * This function should be overriden by a display module (if needed).
 */
void SubredditStats::output(char * msg) {
}


/**
 * A place holder that is called when a message is to be output.
 * This variant accepts a position that the message should be displayed at.
 * This function should be overriden by a display module (if needed).
 */
void SubredditStats::output(char * msg, int line, int col) {
}


/**
 * A place holder that is called when the values on the display should
 * be updated.
 * The parameter is used to indicate that new data has been received.
 * This function should be overriden by a display module (if needed).
 */
void SubredditStats::updateDisplay(boolean newData) {
}
