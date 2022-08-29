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
  "Active: ",
  "Subs: ",
  "New today: ",
  " 7 days: ",
  "30 days: ",
  "90 days: ",
  "Tgt: ",
  "Est: ",
  ""          // Name of the subreddit
};

/**
 * Accept a new (comma seperated) list of values.
 * Seperate the values and remember them.
 */
void SubredditStats::registerInputData(char *statsText) {
  if (strlen(statsText) == 0) {
    return;
  }

  Serial.print("Got: '"); Serial.print(statsText); Serial.println("'");
  strcpy(statsTextBuf, statsText);
  Serial.print("Length: "); Serial.println(strlen(statsText));

  int cnt = 0;
  const char delim[] = ",";
  char *token = strtok(statsTextBuf, delim);
  while (token != NULL) {
    if (cnt < MAX_MSG_CNT) {
      values[cnt] = token;        // Remember the first MAX_MSG_CNT items.
    } else {
      Serial.println("MAX_MSG_CNT overflow");
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
char *SubredditStats::getValue(int itemNo) {
  if (itemNo >= 0 && itemNo < MAX_MSG_CNT) {
    return values[itemNo];
  } else {
    return "";
  }
}


/**
 * Retrieve the standard label corresponding to a metric as indexed by the itemNo.
 */
char *SubredditStats::getLabel(int itemNo) {
  return labels[itemNo];
}

/**
 * A place holder that should be called when a display needs to be initialised.
 */
void SubredditStats::initDisplay() {
}


/**
 * A place holder that is called when a message is to be output.
 */
void SubredditStats::output(char * msg) {
}


/**
 * A place holder that is called when a message is to be output.
 * This variant accepts a position that the message should be displayed at.
 */
void SubredditStats::output(char * msg, int line, int col) {
}


/**
 * A place holder that is called when the values on the display should
 * be updated.
 * The parameter is used to indicate that new data has been received.
 */
void SubredditStats::updateDisplay(boolean newData) {
}
