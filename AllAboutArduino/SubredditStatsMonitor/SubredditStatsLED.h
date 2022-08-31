#ifndef _STATS_DISPLAY_LED
#define _STATS_DISPLAY_LED

#include "SubredditStats.h"

/**
 * Display driver for subreddit metrics on
 * an LED device.
 * It is assumed this device has at least 8 digits.
 */
class SubredditStatsLED : public SubredditStats {
  public:
    void updateDisplay(boolean newData);
    void output(const char * msg);
    void output(const char * msg, int line, int col);
    void initDisplay();

  private:
    unsigned long nextDisplayUpdateTime = 0;
    unsigned long displayUpdateInterval = 1500;\
    void outputNoData();

};

#endif
