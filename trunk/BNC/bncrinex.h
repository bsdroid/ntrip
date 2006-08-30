
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
   void resolveFileName(struct converttimeinfo& cti);
   void readSkeleton();
   void writeHeader(struct converttimeinfo& cti, double second);

   QByteArray          _statID;
   QByteArray          _fName;
   QList<Observation*> _obs;
   ofstream            _out;
   QStringList         _headerLines;
   bool                _headerWritten;
};

#endif
