
#ifndef BNCCOMB_H
#define BNCCOMB_H

#include <fstream>
#include <newmat.h>
#include "bncephuser.h"

class bncRtnetDecoder;
class bncSP3;
class bncAntex;

class cmbParam {
 public:
  enum parType {offAC, offACSat, clkSat};
  cmbParam(parType type_, int index_, const QString& ac_, const QString& prn_);
  ~cmbParam();
  double partial(const QString& AC_, const QString& prn_);
  QString toString() const;
  parType type;
  int     index;
  QString AC;
  QString prn;
  double  xx;
  double  sig0;
  double  sigP;
  bool    epoSpec;
  const t_eph* eph;
};

class bncComb : public bncEphUser  {
 Q_OBJECT

 public:
  bncComb();
  virtual ~bncComb();
  void processCorrLine(const QString& staID, const QString& line);
  int  nStreams() const {return _ACs.size();}

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:

  class cmbAC {
   public:
    cmbAC() {
      weight = 0.0;
      numObs = 0;
    }
    ~cmbAC() {}
    QString  mountPoint;
    QString  name;
    double   weight;
    unsigned numObs;
  };

  class cmbCorr : public t_corr {
   public:
    QString acName;
  };

  class cmbEpoch {
   public:
    cmbEpoch() {}
    ~cmbEpoch() {
      QVectorIterator<cmbCorr*> it(corrs);
      while (it.hasNext()) {
        delete it.next();
      }
    }
    QVector<cmbCorr*> corrs;
  };

  void processEpoch();
  t_irc createAmat(Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP,
                   const ColumnVector& x0, QMap<QString, t_corr*>& resCorr);
  void dumpResults(const QMap<QString, t_corr*>& resCorr);
  void printResults(QTextStream& out, const QMap<QString, t_corr*>& resCorr);
  void switchToLastEph(const t_eph* lastEph, t_corr* corr);
  void switchToLastEph(const t_eph* lastEph, cmbParam* pp);

  QVector<cmbCorr*>& corrs() {return _buffer[_resTime].corrs;}

  QList<cmbAC*>           _ACs;
  bncTime                 _resTime;
  QVector<cmbParam*>      _params;
  QMap<bncTime, cmbEpoch> _buffer;
  bncRtnetDecoder*        _rtnetDecoder;
  SymmetricMatrix         _QQ;
  QByteArray              _log;
  bncAntex*               _antex;
  double                  _MAXRES;
};

#endif
