
#ifndef BNCTABLEDLG_H
#define BNCTABLEDLG_H

#include <QtGui>

class bncTableDlg : public QDialog {
  Q_OBJECT

  public:
    bncTableDlg(QWidget* parent);
    ~bncTableDlg();

  signals:
    void newMountPoints(QStringList* mountPoints);
 
  private slots:
    virtual void accept();
    void slotGetTable();

  private:
    QLineEdit*   _casterHostLineEdit;
    QLineEdit*   _casterPortLineEdit;
    QLineEdit*   _casterUserLineEdit;
    QLineEdit*   _casterPasswordLineEdit;

    QPushButton* _buttonGet;
    QPushButton* _buttonCancel;
    QPushButton* _buttonOK;

    QTableWidget* _table;
};

#endif
