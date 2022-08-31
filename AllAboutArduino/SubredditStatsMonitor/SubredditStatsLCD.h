#ifndef _STATS_DISPLAY_LCD
#define _STATS_DISPLAY_LCD

#include "SubredditStats.h"

/**
 * Display driver for subreddit metrics on
 * an LCD device.
 * It is assumed that this device has at least
 * 2 lines x 16 characters.
 */
class SubredditStatsLCD : public SubredditStats {
  public:
    void updateDisplay(boolean newData);
    void output(const char * msg);
    void output(const char * msg, int line, int col);
    void initDisplay();

  private:
    unsigned long nextDisplayUpdateTime = 0;
    unsigned long displayUpdateInterval = 1500;
};

#endif
