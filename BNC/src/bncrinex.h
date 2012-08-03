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

#include <QtCore>
#include <fstream>

#include "bncconst.h"
#include "rinex/rnxobsfile.h"

class t_obs;

class bncRinex {
 public:
   bncRinex(const QByteArray& statID, const QUrl& mountPoint, 
            const QByteArray& latitude, const QByteArray& longitude,
            const QByteArray& nmea, const QByteArray& ntripVersion); 
   ~bncRinex();
   void deepCopy(t_obs obs);
   void dumpEpoch(const QByteArray& format, long maxTime);
   void setReconnectFlag(bool flag){_reconnectFlag = flag;}
   static QString nextEpochStr(const QDateTime& datTim,
                               const QString& intStr, 
                               QDateTime* nextEpoch = 0);

   int samplingRate() const {return _samplingRate;}

   void setApproxPos(double stax, double stay, double staz) {
     _approxPos[0] = stax;
     _approxPos[1] = stay;
     _approxPos[2] = staz;
   }

   std::string rinexSatLine(const t_obs& obs, char lli1, char lli2, char lli5);

   static std::string asciiSatLine(const t_obs& obs);
   static std::string obsToStr(double val, int width = 14, int precision = 3);

 private:
   void resolveFileName(const QDateTime& datTim);
   void readSkeleton();
   void writeHeader(const QByteArray& format, const QDateTime& datTim, 
                    const QDateTime& datTimNom);
   void closeFile();
   t_irc downloadSkeleton();

   QByteArray    _statID;
   QByteArray    _fName;
   QList<t_obs>  _obs;
   std::ofstream _out;
   bool          _headerWritten;
   QDateTime     _nextCloseEpoch;
   QString       _rnxScriptName;
   QUrl          _mountPoint;
   QString       _pgmName;
   QString       _userName;
   QString       _sklName;
   QByteArray    _latitude;
   QByteArray    _longitude;
   QByteArray    _nmea;
   QByteArray    _ntripVersion;
   bool          _reconnectFlag;
   QDate         _skeletonDate;
   int           _rinexVers;
   double        _approxPos[3];
   int           _samplingRate;

   QMap<QString, int>  _slip_cnt_L1;
   QMap<QString, int>  _slip_cnt_L2;
   QMap<QString, int>  _slip_cnt_L5;

   QMap<char, QVector<QString> > _rnxTypes;

   t_rnxObsHeader _header;
};

#endif
