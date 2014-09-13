
#ifndef BNCCOMB_H
#define BNCCOMB_H

#include <fstream>
#include <newmat.h>
#include "bncephuser.h"
#include "satObs.h"

class bncRtnetDecoder;
class bncSP3;
class bncAntex;

class cmbParam {
 public:
  enum parType {offACgps, offACglo, offACSat, clkSat};
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
  void processOrbCorrections(const QList<t_orbCorr>& orbCorrections);
  void processClkCorrections(const QList<t_clkCorr>& clkCorrections);
  int  nStreams() const {return _ACs.size();}

 public slots:
  void slotProviderIDChanged(QString mountPoint);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newCorrections(QStringList);

 private:

  enum e_method{singleEpoch, filter};

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

  class cmbCorr : public t_clkCorr {
   public:
    QString      acName;
    ColumnVector diffRao;
    QString ID() {return acName + "_" + QString(_prn.toString().c_str());}
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
  t_irc processEpoch_filter(QTextStream& out,
                            QMap<QString, t_clkCorr*>& resCorr,
                            ColumnVector& dx);
  t_irc processEpoch_singleEpoch(QTextStream& out,
                                 QMap<QString, t_clkCorr*>& resCorr,
                                 ColumnVector& dx);
  t_irc createAmat(Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP,
                   const ColumnVector& x0, QMap<QString, t_clkCorr*>& resCorr);
  void dumpResults(const QMap<QString, t_clkCorr*>& resCorr);
  void printResults(QTextStream& out, const QMap<QString, t_clkCorr*>& resCorr);
  void switchToLastEph(const t_eph* lastEph, t_clkCorr* corr);
  t_irc checkOrbits(QTextStream& out);

  QVector<cmbCorr*>& corrs() {return _buffer[_resTime].corrs;}

  t_irc mergeOrbitCorr(const cmbCorr* orbitCorr, cmbCorr* clkCorr);

  QList<cmbAC*>           _ACs;
  bncTime                 _resTime;
  QVector<cmbParam*>      _params;
  QMap<bncTime, cmbEpoch> _buffer;
  bncRtnetDecoder*        _rtnetDecoder;
  SymmetricMatrix         _QQ;
  QByteArray              _log;
  bncAntex*               _antex;
  double                  _MAXRES;
  QString                 _masterOrbitAC;
  unsigned                _masterMissingEpochs;
  e_method                _method;
  bool                    _useGlonass;
  int                     _cmbSampl;
  QMap<QString, cmbCorr*> _orbitCorrs;
};

#endif
