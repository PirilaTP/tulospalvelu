// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
//
// SiritReader: IRfidReader-toteutus Zebra FX9500 (SIRIT) -lukijalle.

#ifndef SIRITREADER_H
#define SIRITREADER_H

#include "IRfidReader.h"

class SiritReader : public IRfidReader {
public:
    int  openConnection(int r_no, bool reconnect);
    bool isConnected(int r_no);
    void readTags(int r_no);
    void sync(bool setTime);
    void readCmd(int r_no);
    int  parseTime(INT32 *t, san_type *vastaus, aikatp *ut, INT *jono, int r_no);
};

#endif // SIRITREADER_H
