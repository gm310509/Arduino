#ifndef _STATS_DISPLAY_LCD
#define _STATS_DISPLAY_LCD

#include "SubredditStats.h"

class SubredditStatsLCD : public SubredditStats {
  public:
    void updateDisplay(boolean newData);
    void output(char * msg);
    void output(char * msg, int line, int col);
    void initDisplay();

  private:
    unsigned long nextDisplayUpdateTime = 0;
    unsigned long displayUpdateInterval = 1500;
};

#endif
