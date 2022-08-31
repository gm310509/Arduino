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
#ifndef _SUBREDDIT_STATS
#define _SUBREDDIT_STATS

#define BUF_SIZE  80
#define MAX_MSG_CNT 9

class SubredditStats {
  public:
    /**
     * 
     * process new data received from the data source.
     * @param statsText variable contains the CSV data
     *                  obtained from the proxy.
     */
    virtual void registerInputData(char *statsText) final;
    /**
     * @return the flag indicating whether data has been received or not.
     */
    virtual boolean hasValidData() final {
      return _hasValidData;
    }
    /**
     * Retrieve a metric as a string of characters.
     * @param the id of the value to be retrieved.
     * @return the metric as a <em>string</em> of characters.
     */
    virtual const char *getValue(int itemNo) final;
    /**
     * Retrieve a standard label associated with a metric.
     * The default labels are typically fairly short.
     * @param the id of the value to be retrieved.
     * @return a default label for the metric.
     */
    virtual const char *getLabel(int itemNo) final;

    /***************  Display function placeholders **********/
    /**
     * Called when the display should be initialised
     */
    virtual void initDisplay();
    /**
     * Cause a message to be displayed.
     */
    virtual void output(char * msg);
    /**
     * Cause a message to be displayed at a specified location
     */
    virtual void output(char * msg, int line, int col);
    /**
     * Cause the display contents to be updated.
     * This is the main "display data" function.
     * It is called frequently to allow any animation / cycling
     * and when new data is received.
     * 
     * @param newData - true: indicates that new data has been received.
     *                  false: no new data, but provides an opportunity for
     *                         the display to be modified with the existing
     *                         data.
     */
    virtual void updateDisplay(boolean newData);

  protected:
    int messageIndex = 0;           // Track the metric/message being displayed
  
  private:
    char statsTextBuf[BUF_SIZE];    // A buffer to hold the metrics data.
    char *values[MAX_MSG_CNT];      // Pointers to the individual metrics in the buffer.
    boolean _hasValidData = false;  // A latch that tracks if data has been received.
};

#endif
