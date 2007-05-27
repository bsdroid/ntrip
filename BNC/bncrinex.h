// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
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

#include <QByteArray>
#include <QDateTime>
#include <QList>

#include <fstream>

#include "bncconst.h"
#include "RTCM/GPSDecoder.h"

class bncRinex {
 public:
   bncRinex(const QByteArray& statID, const QUrl& mountPoint, 
            const QByteArray& format, const QByteArray& latitude,
            const QByteArray& longitude, const QByteArray& nmea);
   ~bncRinex();
   void deepCopy(const Observation* obs);
   void dumpEpoch(long maxTime);
   void setReconnectFlag(bool flag){_reconnectFlag = flag;}

 private:
   void resolveFileName(const QDateTime& datTim);
   void readSkeleton();
   void writeHeader(const QDateTime& datTim, const QDateTime& datTimNom);
   void closeFile();
   t_irc downloadSkeleton();

   QByteArray          _statID;
   QByteArray          _fName;
   QList<Observation*> _obs;
   std::ofstream       _out;
   QStringList         _headerLines;
   bool                _headerWritten;
   QDateTime           _nextCloseEpoch;
   QString             _rnxScriptName;
   QUrl                _mountPoint;
   QString             _pgmName;
   QString             _userName;
   QString             _sklName;
   QByteArray          _format;
   QByteArray          _latitude;
   QByteArray          _longitude;
   QByteArray          _nmea;
   bool                _reconnectFlag;
   QDate               _skeletonDate;
   QMutex              _mutex;
};

#endif
