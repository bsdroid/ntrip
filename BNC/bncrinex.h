
#ifndef BNCRINEX_H
#define BNCRINEX_H

#include <QByteArray>
#include <QList>

#include <fstream>

#include "RTCM/GPSDecoder.h"

class bncRinex {
 public:
   bncRinex(const char* StatID);
   ~bncRinex();
   void deepCopy(const Observation* obs);
   void dumpEpoch();

 private:
   void writeHeader();

   QByteArray          _statID;
   QList<Observation*> _obs;
   ofstream            _out;
   bool                _headerWritten;
};

#endif
