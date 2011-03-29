
#ifndef BNCCOMB_H
#define BNCCOMB_H

#include <fstream>
#include <newmat.h>
#include "bncephuser.h"

class cmbCaster;
class bncSP3;
class bncAntex;

class cmbParam {
 public:
  enum parType {AC_offset, Sat_offset, clk};
  cmbParam(parType type_, int index_, const QString& ac_, 
           const QString& prn_, double sig_0_, double sig_P_);
  ~cmbParam();
  double partial(const QString& AC_, t_corr* corr);
  QString toString() const;
  parType type;
  int     index;
  QString AC;
  QString prn;
  double  xx;
  double  sig_0;
  double  sig_P;
};

class bncComb : public bncEphUser  {
 Q_OBJECT

 public:
  bncComb();
  ~bncComb();
  void processCorrLine(const QString& staID, const QString& line);
  int  nStreams() const {return _ACs.size();}

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:
  class cmbEpoch {
   public:
    cmbEpoch(const QString& name) {acName = name;}
    ~cmbEpoch() {
      QMapIterator<QString, t_corr*> it(corr);
      while (it.hasNext()) {
        it.next();
        delete it.value();
      }
    }
    QString                acName;
    bncTime                time;
    QMap<QString, t_corr*> corr; // Corrections (key is PRN)
  };

  class cmbAC {
   public:
    cmbAC() {}
    ~cmbAC() {
      QListIterator<cmbEpoch*> it(epochs);
      while (it.hasNext()) {
        delete it.next();
      }
    }
    QString           mountPoint;
    QString           name;
    double            weight;
    QQueue<cmbEpoch*> epochs;  // List of Epochs with Corrections
  };

  void processEpochs(const QList<cmbEpoch*>& epochs);
  void dumpResults(const bncTime& resTime, 
                   const QMap<QString, t_corr*>& resCorr);
  void printResults(QTextStream& out, const bncTime& resTime,
                    const QMap<QString, t_corr*>& resCorr);
  void switchToLastEph(const t_eph* lastEph, t_corr* corr);

  QMap<QString, cmbAC*> _ACs;   // Analytical Centers (key is mountpoint)
  bncTime               _processedBeforeTime;
  cmbCaster*            _caster;
  QVector<cmbParam*>    _params;
  SymmetricMatrix       _QQ;
  bool                  _firstReg;
  QByteArray            _log;
  QString               _masterAC;
  QString               _outNameSkl;
  QString               _outName;
  std::ofstream*        _out;
  bncSP3*               _sp3;
  bool                  _append;
  bncAntex*             _antex;
};

#endif
