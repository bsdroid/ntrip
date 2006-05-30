
#ifndef BNCTABLEDLG_H
#define BNCTABLEDLG_H

#include <QtGui>

class bncTableDlg : public QDialog {
  Q_OBJECT

  public:
    bncTableDlg(QWidget* parent, const QString& proxyHost, int proxyPort);
    ~bncTableDlg();

  signals:
    void newMountPoints(QStringList* mountPoints);

  private slots:
    virtual void accept();
    void slotGetTable();

  private:
    QString      _proxyHost;
    int          _proxyPort;

    QLabel*      _casterHostLabel;
    QLabel*      _casterPortLabel;
    QLineEdit*   _casterHostLineEdit;
    QLineEdit*   _casterPortLineEdit;

    QPushButton* _buttonGet;
    QPushButton* _buttonCancel;
    QPushButton* _buttonOK;

    QTableWidget* _table;
};

#endif
