
#ifndef BNCRINEX_H
#define BNCRINEX_H

#include <QProcess>
#include <QByteArray>
#include <QDateTime>
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
   void resolveFileName(const QDateTime& datTim);
   void readSkeleton();
   void writeHeader(const QDateTime& datTim);
   void closeFile();

   QByteArray          _statID;
   QByteArray          _fName;
   QList<Observation*> _obs;
   ofstream            _out;
   QStringList         _headerLines;
   bool                _headerWritten;
   QDateTime           _nextCloseEpoch;
   QString             _rnxScriptName;
   QProcess            _rnxScript;
   QString             _mountPoint;
   QString             _pgmName;
   QString             _userName;
};

#endif
