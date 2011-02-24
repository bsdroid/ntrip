
#ifndef BNCCOMB_H
#define BNCCOMB_H

#include <newmat.h>
#include "bncephuser.h"

class cmbCaster;

class cmbParam {
 public:
  enum parType {AC_offset, Sat_offset, clk};
  cmbParam(parType typeIn, int indexIn, 
           const QString& acIn, const QString& prnIn);
  ~cmbParam();
  double partial(const QString& acIn, t_corr* corr);
  QString toString() const;
  parType type;
  int     index;
  QString AC;
  QString prn;
  double  xx;
  int     iod;
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
  void switchToLastEph(const QString& ACname, const t_eph* lastEph, 
                       const t_eph* prevEph, t_corr* newCorr);

  QMap<QString, cmbAC*> _ACs;   // Analytical Centers (key is mountpoint)
  bncTime               _processedBeforeTime;
  cmbCaster*            _caster;
  QVector<cmbParam*>    _params;
  double                _sigACOff;
  double                _sigSatOff;
  double                _sigClk;
  SymmetricMatrix       _QQ;
  QByteArray            _log;
};

#endif
