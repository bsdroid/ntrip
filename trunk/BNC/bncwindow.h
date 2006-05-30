
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

  private slots:
    void slotSaveOptions();
    void slotAddMountPoints();
    void slotGetData();
    void slotNewMountPoints(QStringList* mountPoints);
    void slotDeleteMountPoints();
    void slotGetThreadErrors();

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
    QLabel*    _mountPointsLabel;
    QLabel*    _outFileLabel;
    QLabel*    _outPortLabel;

    QLineEdit* _proxyHostLineEdit;
    QLineEdit* _proxyPortLineEdit;
    QLineEdit* _userLineEdit;
    QLineEdit* _passwordLineEdit;
    QLineEdit* _outFileLineEdit;
    QLineEdit* _outPortLineEdit;

    QWidget*   _canvas;

    QTableWidget* _mountPointsTable;

    bncCaster* _bncCaster;
};
#endif
