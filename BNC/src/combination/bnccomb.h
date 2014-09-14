
#ifndef BNCCOMB_H
#define BNCCOMB_H

#include <fstream>
#include <newmat.h>
#include "bncephuser.h"
#include "satObs.h"

class bncRtnetDecoder;
class bncSP3;
class bncAntex;

class bncComb : public bncEphUser  {
 Q_OBJECT
 public:
  bncComb();
  virtual ~bncComb();
  int  nStreams() const {return _ACs.size();}

 public slots:
  void slotProviderIDChanged(QString mountPoint);
  void slotNewOrbCorrections(QList<t_orbCorr> orbCorrections);
  void slotNewClkCorrections(QList<t_clkCorr> clkCorrections);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newClkCorrections(QList<t_clkCorr> clkCorrections);
  void newOrbCorrections(QList<t_orbCorr> orbCorrections);

 private:
  enum e_method{singleEpoch, filter};

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

  class cmbCorr {
   public:
    cmbCorr() {
      _eph     = 0;
      _iod     = 0;
      _orbCorr = 0;
      _clkCorr = 0;
    }
    ~cmbCorr() {
      delete _orbCorr;
      delete _clkCorr;
    }
    QString      _prn;
    bncTime      _time;
    int          _iod;
    const t_eph* _eph;
    t_orbCorr*   _orbCorr; // used for input
    t_clkCorr*   _clkCorr; // used for input
    QString      _acName; 
    double       _dClk;    // used for output
    ColumnVector _diffRao;
    QString ID() {return _acName + "_" + _prn;}
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

  void  processEpoch();
  t_irc processEpoch_filter(QTextStream& out, QMap<QString, cmbCorr*>& resCorr,
                            ColumnVector& dx);
  t_irc processEpoch_singleEpoch(QTextStream& out, QMap<QString, cmbCorr*>& resCorr,
                                 ColumnVector& dx);
  t_irc createAmat(Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP,
                   const ColumnVector& x0, QMap<QString, cmbCorr*>& resCorr);
  void  dumpResults(const QMap<QString, cmbCorr*>& resCorr);
  void  printResults(QTextStream& out, const QMap<QString, cmbCorr*>& resCorr);
  void  switchToLastEph(const t_eph* lastEph, cmbCorr* corr);
  t_irc checkOrbits(QTextStream& out);
  QVector<cmbCorr*>& corrs() {return _buffer[_resTime].corrs;}

  QList<cmbAC*>                          _ACs;
  bncTime                                _resTime;
  QVector<cmbParam*>                     _params;
  QMap<bncTime, cmbEpoch>                _buffer;
  bncRtnetDecoder*                       _rtnetDecoder;
  SymmetricMatrix                        _QQ;
  QByteArray                             _log;
  bncAntex*                              _antex;
  double                                 _MAXRES;
  QString                                _masterOrbitAC;
  unsigned                               _masterMissingEpochs;
  e_method                               _method;
  bool                                   _useGlonass;
  int                                    _cmbSampl;
  QMap<QString, QMap<t_prn, t_orbCorr> > _orbCorrections;
};

#endif
