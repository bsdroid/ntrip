
#ifndef BNCTABLEDLG_H
#define BNCTABLEDLG_H

#include <QtCore>
#include <QtGui>

#include "bncconst.h"

class bncTableDlg : public QDialog {
  Q_OBJECT

  public:
    bncTableDlg(QWidget* parent);
    ~bncTableDlg();
    static t_irc getFullTable(const QString& casterHost, int casterPort,
                              QStringList& allLines);

  signals:
    void newMountPoints(QStringList* mountPoints);

  private slots:
    virtual void accept();
    void slotGetTable();
    void slotSkl();
    void slotSelectionChanged();

  private:
    QLineEdit*   _casterHostLineEdit;
    QLineEdit*   _casterPortLineEdit;
    QLineEdit*   _casterUserLineEdit;
    QLineEdit*   _casterPasswordLineEdit;

    QPushButton* _buttonSkl;
    QPushButton* _buttonGet;
    QPushButton* _buttonCancel;
    QPushButton* _buttonOK;

    QTableWidget* _table;
};

#endif
