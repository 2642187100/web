#ifndef __CLIBRINGBUF_H__
#define __CLIBRINGBUF_H__

#include "clibCommon.h"


#define RING_BUFFER_SIZE   (1024*1024*2)
#define RING_BUFFER_MASK   (RING_BUFFER_SIZE - 1)


class clibRingbuf
{
public:
    clibRingbuf(int bufsize = RING_BUFFER_SIZE);
    ~clibRingbuf();


private:
    char mp_buf[RING_BUFFER_SIZE] = {0,};
    char *mp_bufstart = nullptr;
    char *mp_bufend   = nullptr;

    char *mp_r = nullptr;
    char *mp_tail  = nullptr;
    char *mp_w = nullptr;

    int m_availablecnt = 0;
    int m_cntpercap    = 0;
    int m_ringbufsize = 0;


public:
    char* buf() {
        return mp_buf;
    }

    void status();


public:
    virtual int getLenPos() {
        return 0;
    }

    virtual int getLen(unsigned char *val) {
        Q_UNUSED(val);

        return 0;
    }


    inline int isFull();
    int getAvailableCnt();
    inline int getFreeCnt();
    char* getBuf(int size);
    char* updatePtr(int size);
    char* deqeueChar(char *val);
    int searchFlag(unsigned char flag);
    int deqeueChars(char* pbuf, int size);
    int seekN(int size, unsigned char *val);
    int searchCopy(char flag, char *pbuf, int size);

    void ringBufReset();
};

#endif

