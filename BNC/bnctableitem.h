
#ifndef BNCTABLEITEM_H
#define BNCTABLEITEM_H

#include <QtCore>
#include <QtGui>

struct Observation;

class bncTableItem : public QObject, public QTableWidgetItem {
  Q_OBJECT

  public:
    bncTableItem();
    ~bncTableItem();

  signals:
 
  public slots:
    void slotNewObs(const QByteArray& staID, Observation* obs);

  private:
    double _bytesRead;
};

#endif
