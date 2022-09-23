#ifndef _STATS_DISPLAY_TFT_8230
#define _STATS_DISPLAY_TFT_8230

#include "SubredditStats.h"

class SubredditStatsTFT : public SubredditStats {
  public:
    void updateDisplay(boolean newData);
    void output(const char * msg);
    void output(const char * msg, int line, int col);
    void initDisplay();

  private:
    unsigned long nextDisplayUpdateTime = 0;
    unsigned long displayUpdateInterval = 1500;
    uint16_t displayWidth = 0;
    uint16_t displayHeight = 0;

    void center(const char * msg);
    void outputPoint(int16_t x, int16_t y, boolean newLine);
    void getWidthAndHeight(const char * msg, uint16_t *w, uint16_t *h);
    void drawBorder(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void drawBorder();

};

#endif
