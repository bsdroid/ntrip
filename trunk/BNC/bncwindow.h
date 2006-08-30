
#ifndef BNCWINDOW_H
#define BNCWINDOW_H

#include <QtGui>

#include "bncgetthread.h"
#include "bnccaster.h"

class bncWindow : public QMainWindow {
  Q_OBJECT

  public:
    bncWindow();
    ~bncWindow();
  
  public slots:  
    void slotMessage(const QByteArray& msg);

  private slots:
    void slotSaveOptions();
    void slotAddMountPoints();
    void slotGetData();
    void slotNewMountPoints(QStringList* mountPoints);
    void slotDeleteMountPoints();
    void slotGetThreadErrors();
    void slotSelectionChanged();

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
    QMenu*     _menuHlp;
    QMenu*     _menuFile;

    QAction*   _actAbout;
    QAction*   _actSaveOpt;
    QAction*   _actQuit; 
    QAction*   _actGetData;
    QAction*   _actAddMountPoints;
    QAction*   _actDeleteMountPoints;

    QLabel*    _proxyHostLabel;
    QLabel*    _proxyPortLabel;
    QLabel*    _userLabel;
    QLabel*    _passwordLabel;
    QLabel*    _rnxPathLabel;
    QLabel*    _rnxSkelLabel;
    QLabel*    _rnxIntrLabel;
    QLabel*    _mountPointsLabel;
    QLabel*    _outFileLabel;
    QLabel*    _outPortLabel;
    QLabel*    _logLabel;

    QLineEdit* _proxyHostLineEdit;
    QLineEdit* _proxyPortLineEdit;
    QLineEdit* _userLineEdit;
    QLineEdit* _passwordLineEdit;
    QLineEdit* _outFileLineEdit;
    QLineEdit* _outPortLineEdit;
    QLineEdit* _rnxPathLineEdit;
    QLineEdit* _rnxSkelLineEdit;
    QSpinBox*  _rnxIntrSpinBox;
    QTableWidget* _mountPointsTable;

    QTextEdit*  _log;

    QWidget*   _canvas;

    bncCaster* _bncCaster;
};
#endif
