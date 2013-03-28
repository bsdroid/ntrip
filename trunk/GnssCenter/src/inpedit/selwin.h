#ifndef GnssCenter_SELWIN_H
#define GnssCenter_SELWIN_H

#include <QtGui>

namespace GnssCenter {

class t_selWin : public QWidget {
  Q_OBJECT

  Q_ENUMS( Mode )
  Q_PROPERTY( Mode mode READ mode WRITE setMode )
  Q_PROPERTY( QString fileName READ fileName WRITE setFileName )

 public:
  enum Mode {File, Files, Directory};

  t_selWin(QWidget* parent = 0, t_selWin::Mode mode = File);
  ~t_selWin();

  QString fileName() const;
  Mode mode() const {return _mode;}

  public slots:
   void setFileName(const QString& fileName);
   void setMode(Mode mode) {_mode = mode;}

  signals:
   void fileNameChanged(const QString&);

  private slots:
   void chooseFile();

  private:
   QLineEdit*   _lineEdit;
   QPushButton* _button;
   Mode         _mode;
};

} // namespace GnssCenter

#endif
