#include "clibRingbuf.h"

clibRingbuf::clibRingbuf(int bufsize)
{
    int ringbufsize = RING_BUFFER_SIZE > bufsize ? RING_BUFFER_SIZE : bufsize;

    mp_bufstart    = mp_buf;
    mp_bufend      = mp_buf + ringbufsize;
    m_ringbufsize  = ringbufsize;

    mp_r     = mp_buf;
    mp_tail  = mp_bufend;
    mp_w     = mp_buf;

    m_availablecnt = 0;
}


clibRingbuf::~clibRingbuf()
{
    CLIBPD("~clibRingbuf");

    CLIBPD("~clibRingbuf over");
}


void clibRingbuf::status()
{
    CLIBPD("buffer infos");
    CLIBPD("     ringbuf size: %d",   m_ringbufsize);
    CLIBPD("   buf start addr: 0x%x", mp_bufstart);
    CLIBPD("     buf end addr: 0x%x", mp_bufend);

    CLIBPD("       write addr: 0x%x", mp_w);

    CLIBPD("search start addr: 0x%x", mp_r);
    CLIBPD("search tail  addr: 0x%x", mp_tail);
}


inline int clibRingbuf::isFull()
{
    return ((mp_r - mp_w) & RING_BUFFER_MASK) == RING_BUFFER_MASK;
}


int clibRingbuf::getAvailableCnt()
{
    int cnt = ( ((mp_w - mp_r) & RING_BUFFER_MASK) - (mp_bufend - mp_tail) );

    if (cnt < 0) {
        CLIBPW("there must be some error.");
        cnt = 0;

    } else if (cnt > m_ringbufsize) {
        CLIBPW("there must be some error.");
        cnt = m_ringbufsize;
    }

    return cnt;
}


inline int clibRingbuf::getFreeCnt()
{
    int occupycnt = getAvailableCnt();

    return m_ringbufsize - occupycnt;
}


char* clibRingbuf::getBuf(int size)
{
    if (mp_w >= mp_r) { //no swap
        if (mp_tail - mp_w >= size) { //left space enough
            return mp_w;

        } else {                      //left space not enough, swap to header
            if (mp_r - mp_bufstart >= size) {   //header-->mp_r space enough
                mp_tail = mp_w;
                mp_w = mp_bufstart;
                return mp_w;
            } else {                            //header-->mp_r space not enough
                CLIBPD("no enought space, need %d, free %d, available %d, mp_r 0x%x, mp_w 0x%x, mp_tail 0x%x, bufstart 0x%x, bufend 0x%x",
                       size, getFreeCnt(), getAvailableCnt(), mp_r, mp_w, mp_tail, mp_bufstart, mp_bufend);
                return nullptr;
            }
        }

    } else {            //has swap
        if (mp_r - mp_w >= size) {    //space enough
            return mp_w;
        } else {
            CLIBPD("no enought space, need %d, free %d, available %d, mp_r 0x%x, mp_w 0x%x, mp_tail 0x%x, bufstart 0x%x, bufend 0x%x",
                   size, getFreeCnt(), getAvailableCnt(), mp_r, mp_w, mp_tail, mp_bufstart, mp_bufend);
            return nullptr;
        }
    }
}


//space is enough, so just add size
char* clibRingbuf::updatePtr(int size)
{
    mp_w += size;

    return mp_w;
}


char* clibRingbuf::deqeueChar(char* val)
{
    *val = *mp_r;

    if (mp_tail > mp_r) {
        mp_r++;

    } else if (mp_tail == mp_r) {  //reach serach end, swap mp_r and move mp_tail to buffer end
        mp_r    = mp_bufstart;
        mp_tail = mp_bufend;
    }

    return mp_r;
}


int clibRingbuf::seekN(int size, unsigned char *val)
{
    unsigned char *p = (unsigned char *)mp_r;
    int avaicnt = getAvailableCnt();

    if (avaicnt <= 0) {
        CLIBPD("avaicnt is %d", avaicnt);
        return 0;
    }

    if (avaicnt < size) {
        CLIBPD("avaicnt is %d, size is %d", avaicnt, size);
        return 0;
    }

    while ( size-- > 0 ) {
        if ((unsigned char *)mp_tail > p) {
            p++;

        } else if ((unsigned char *)mp_tail == p) {  //reach serach end, swap mp_r and move mp_tail to buffer end
            p = (unsigned char *)mp_bufstart;
        }
    }

    *val = *p;

    return 1;
}


int clibRingbuf::searchFlag(unsigned char flag)
{
    char val = 0;

    //qDebug("searchflag getavailablecnt is %d\n", getAvailableCnt());

    while ( getAvailableCnt() > 0 ) {
        //qDebug("%x  ", (unsigned char)(*mp_r));
        if ((unsigned char)(*mp_r) == flag) {
            //CLIBPD("find flag %d", flag);
            return 1;
        }

        deqeueChar(&val);
    }

    return 0;
}


int clibRingbuf::deqeueChars(char* pbuf, int size)
{
    int cnt = 0;
    char* p = pbuf;

    if ( getAvailableCnt() < size ) {
        CLIBPD("available %d, size %d", getAvailableCnt(), size);
        return 0;
    }

    while (cnt < size) {
        if (mp_tail > mp_r) {
            *p = *mp_r;

            p++;
            mp_r++;

        } else if (mp_tail == mp_r) {  //reach serach end, swap mp_r and move mp_tail to buffer end
            mp_r    = mp_bufstart;
            mp_tail = mp_bufend;

            *p = *mp_r;

            p++;

        } else { //mp_tail will not less than mp_r

        }

        cnt++;
    }

    return cnt;
}


int clibRingbuf::searchCopy(char flag, char *pbuf, int size)
{
    int ret = 0;
    int lenpos = 0;
    unsigned char val = 0;
    int len = 0;

    //qDebug("head is 0x%x\n", flag);

    ret = searchFlag(flag);
    if (ret) {
        lenpos = getLenPos();
        //qDebug("length pos is 0x%x\n", lenpos);

        if (seekN(lenpos, &val) ) {
            len = getLen(&val);
            //qDebug("length is 0x%x\n", len);
        }

        if ( getAvailableCnt() >= len ) {
            //qDebug("had enough data %d\n", getAvailableCnt());
            return deqeueChars(pbuf, len);

        } else {
            //CLIBPD("not enough data, wait next.");
        }
    }

    return 0;
}


void clibRingbuf::ringBufReset()
{
    mp_bufstart = mp_buf;
    mp_bufend   = mp_buf + m_ringbufsize;

    mp_r     = mp_buf;
    mp_tail  = mp_bufend;
    mp_w     = mp_buf;

    m_cntpercap = 0;
    m_availablecnt = 0;
}


