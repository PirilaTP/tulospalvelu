// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
//
// ZebraReader: IRfidReader-toteutus Zebra FX9600 -lukijalle (LLRP).

#ifndef ZEBRAREADER_H
#define ZEBRAREADER_H

#include "IRfidReader.h"

class ZebraReader : public IRfidReader {
public:
    int  openConnection(int r_no, bool reconnect);
    bool isConnected(int r_no);
    void readTags(int r_no);
    void sync(bool setTime);
    void readCmd(int r_no);
    int  parseTime(INT32 *t, san_type *vastaus, aikatp *ut, INT *jono, int r_no);
};

#endif // ZEBRAREADER_H
