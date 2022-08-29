#ifndef _STATS_DISPLAY_LED
#define _STATS_DISPLAY_LED

#include "SubredditStats.h"

class SubredditStatsLED : public SubredditStats {
  public:
    void updateDisplay(boolean newData);
    void output(char * msg);
    void output(char * msg, int line, int col);
    void initDisplay();

  private:
    unsigned long nextDisplayUpdateTime = 0;
    unsigned long displayUpdateInterval = 1500;\
    void outputNoData();

};

#endif
