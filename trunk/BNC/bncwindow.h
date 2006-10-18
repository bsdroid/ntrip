
#ifndef BNCWINDOW_H
#define BNCWINDOW_H

#include <QtGui>

#include "bncgetthread.h"

class bncWindow : public QMainWindow {
  Q_OBJECT

  public:
    bncWindow();
    ~bncWindow();
  
  public slots:  
    void slotMessage(const QByteArray& msg);

  private slots:
    void slotHelp();
    void slotAbout();
    void slotFontSel();
    void slotSaveOptions();
    void slotAddMountPoints();
    void slotGetData();
    void slotStop();
    void slotNewMountPoints(QStringList* mountPoints);
    void slotDeleteMountPoints();
    void slotGetThreadErrors();
    void slotSelectionChanged();

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
    QMenu*     _menuHlp;
    QMenu*     _menuFile;

    QAction*   _actHelp;
    QAction*   _actAbout;
    QAction*   _actFontSel;
    QAction*   _actSaveOpt;
    QAction*   _actQuit; 
    QAction*   _actGetData;
    QAction*   _actStop;
    QAction*   _actAddMountPoints;
    QAction*   _actDeleteMountPoints;

    QLineEdit* _proxyHostLineEdit;
    QLineEdit* _proxyPortLineEdit;
    QLineEdit* _outFileLineEdit;
    QLineEdit* _outPortLineEdit;
    QLineEdit* _rnxPathLineEdit;
    QLineEdit* _rnxSkelLineEdit;
    QLineEdit* _rnxScrpLineEdit;
    QLineEdit* _logFileLineEdit;
    QComboBox* _rnxIntrComboBox;
    QSpinBox*  _rnxSamplSpinBox;
    QCheckBox* _rnxAppendCheckBox;
    QSpinBox*  _waitTimeSpinBox;
    QTableWidget* _mountPointsTable;

    QTextEdit*  _log;

    QWidget*   _canvas;
};
#endif
