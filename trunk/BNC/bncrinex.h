// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters,
// written by Leos Mervart.
//
// Copyright (C) 2006
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Advanced Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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
