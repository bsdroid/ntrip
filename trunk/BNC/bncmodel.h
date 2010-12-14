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

#ifndef BNCMODEL_H
#define BNCMODEL_H

#include <QtCore>
#include <QtNetwork>
#include <newmat.h>

#include "bncconst.h"
#include "bnctime.h"

class t_epoData;
class t_satData;

class bncParam {
 public:
  enum parType {CRD_X, CRD_Y, CRD_Z, RECCLK, TROPO, AMB_L3, GALILEO_OFFSET};
  bncParam(parType typeIn, int indexIn, const QString& prn);
  ~bncParam();
  double partial(t_satData* satData, bool phase);
  bool isCrd() const {
    return (type == CRD_X || type == CRD_Y || type == CRD_Z);
  }
  parType  type;
  double   xx;
  int      index;
  int      index_old;
  QString  prn;
};

class bncModel : public QObject {
 Q_OBJECT
 public:
  bncModel(QByteArray staID);
  ~bncModel();
  t_irc cmpBancroft(t_epoData* epoData);
  t_irc update(t_epoData* epoData);
  bncTime time()  const {return _time;}
  double x()      const {return _params[0]->xx;}
  double y()      const {return _params[1]->xx;}
  double z()      const {return _params[2]->xx;}
  double clk()    const {return _params[3]->xx;}
  double trp() const {
    for (int ii = 0; ii < _params.size(); ++ii) {
      bncParam* pp = _params[ii];
      if (pp->type == bncParam::TROPO) {
        return pp->xx;
      }
    }
    return 0.0;
  }
  double Galileo_offset() const {
    for (int ii = 0; ii < _params.size(); ++ii) {
      bncParam* pp = _params[ii];
      if (pp->type == bncParam::GALILEO_OFFSET) {
        return pp->xx;
      }
    }
    return 0.0;
  }

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newNMEAstr(QByteArray str);

 private:
  void   cmpEle(t_satData* satData);
  void   addAmb(t_satData* satData);
  void   addObs(unsigned& iObs, t_satData* satData,
                Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP);
  void   printRes(const ColumnVector& vv, 
                  std::ostringstream& str, t_satData* satData);
  void   findMaxRes(const ColumnVector& vv,
                    const QMap<QString, t_satData*>& satData,
                    QString& prnCode,  double& maxResCode, 
                    QString& prnPhase, double& maxResPhase);
  double cmpValue(t_satData* satData, bool phase);
  double delay_saast(double Ele);
  void   predict(t_epoData* epoData);
  int    outlierDetection(const SymmetricMatrix& QQsav, 
                          const ColumnVector& vv,
                          QMap<QString, t_satData*>& satDataGPS,
                          QMap<QString, t_satData*>& satDataGlo,
                          QMap<QString, t_satData*>& satDataGal);
  void writeNMEAstr(const QString& nmStr);

  static void kalman(const Matrix& AA, const ColumnVector& ll, 
                     const DiagonalMatrix& PP, 
                     SymmetricMatrix& QQ, ColumnVector& dx);

  double windUp(const QString& prn, const ColumnVector& rSat,
                const ColumnVector& rRec);

  QDateTime  _startTime;

  class pppPos {
   public:
    pppPos() {
      for (int ii = 0; ii < 7; ++ii) {
        xnt[ii] = 0.0;
      }
    }
    bncTime time;
    double  xnt[7];
  };

  bncTime               _time;
  QByteArray            _staID;
  QVector<bncParam*>    _params;
  SymmetricMatrix       _QQ;
  ColumnVector          _xcBanc;
  ColumnVector          _ellBanc;
  bool                  _usePhase;
  bool                  _estTropo;
  QByteArray            _log;
  QFile*                _nmeaFile;
  QTextStream*          _nmeaStream;
  bool                  _useGlonass;
  bool                  _useGalileo;
  QMap<QString, double> _windUpTime;
  QMap<QString, double> _windUpSum;
  QVector<pppPos*>      _posAverage;
  double                _quickStart;
  double                _sigCrd0;
  double                _sigCrdP;
  double                _sigTrp0;
  double                _sigTrpP;
  double                _sigGalileoOffset0;
  double                _sigGalileoOffsetP;
  double                _sigL3;
  double                _sigP3;
  double                _sigAmb0;
  double                _sigClk0;
};

#endif
