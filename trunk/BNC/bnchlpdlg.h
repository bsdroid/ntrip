
#ifndef BNCHLPDLG_H
#define BNCHLPDLG_H

#include <QtCore>
#include <QtGui>

class bncHtml;

class bncHlpDlg : public QDialog {
  Q_OBJECT

  public:
    bncHlpDlg(QWidget* parent, const QUrl& url);
    ~bncHlpDlg();

  signals:
 
  public slots:
    void backwardAvailable(bool);
    void forwardAvailable(bool);

  private:
    bncHtml*     _tb;
    QPushButton* _backButton;
    QPushButton* _forwButton;
    QPushButton* _closeButton;
};

#endif
