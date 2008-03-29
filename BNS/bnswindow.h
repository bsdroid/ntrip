#ifndef BNSWINDOW_H
#define BNSWINDOW_H

#include <QtGui>

class bnsWindow : public QMainWindow {
Q_OBJECT

 public:
  bnsWindow();
  ~bnsWindow();

 public slots:  

 private slots:

 protected:
   virtual void closeEvent(QCloseEvent *);

 private:

};
#endif
