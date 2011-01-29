
#ifndef BNCCOMB_H
#define BNCCOMB_H

#include "bncephuser.h"

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
    cmbEpoch() {}
    ~cmbEpoch() {
      QMapIterator<QString, t_corr*> it(corr);
      while (it.hasNext()) {
        it.next();
        delete it.value();
      }
    }
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

  void processEpochs();
  void processSingleCorr(const cmbAC* AC, const t_corr* corr);
  void printResults() const;

  QMap<QString, cmbAC*> _ACs;   // Analytical Centers (key is mountpoint)
  bncTime               _processedBeforeTime;
};

#endif
