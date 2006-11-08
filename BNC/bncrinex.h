
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
   bncRinex(const char* StatID, const QUrl& mountPoint);
   ~bncRinex();
   void deepCopy(const Observation* obs);
   void dumpEpoch(long maxTime);

 private:
   void resolveFileName(const QDateTime& datTim);
   void readSkeleton();
   void writeHeader(const QDateTime& datTim, const QDateTime& datTimNom);
   void closeFile();

   QByteArray          _statID;
   QByteArray          _fName;
   QList<Observation*> _obs;
   std::ofstream       _out;
   QStringList         _headerLines;
   bool                _headerWritten;
   QDateTime           _nextCloseEpoch;
   QString             _rnxScriptName;
   QProcess            _rnxScript;
   QUrl                _mountPoint;
   QString             _pgmName;
   QString             _userName;
   QString             _sklName;
};

#endif
