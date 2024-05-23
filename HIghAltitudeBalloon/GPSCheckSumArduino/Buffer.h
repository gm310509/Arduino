#ifndef _BUFFER_H
#define _BUFFER_H


#ifndef BUFFER_SIZE
#define BUFFER_SIZE 80
#endif

class Buffer {
  public:
    // Update methods
    boolean append(char ch);
    boolean remove(const unsigned int cnt = 1);
    void clear();
    void reset() { clear(); }

    // Information request methods.
    int getLen();
    const char * getMsg();

  private:
    unsigned int bufPtr = 0;
    char buf[BUFFER_SIZE];
};

#endif
