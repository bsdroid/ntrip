
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_selWin
 *
 * Purpose:    Widget for File/Directory Selection
 *
 * Author:     L. Mervart
 *
 * Created:    08-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "selwin.h"

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////////
t_selWin::t_selWin(QWidget* parent, t_selWin::Mode mode) : 
  QWidget(parent), _mode(mode) {

  QHBoxLayout* layout = new QHBoxLayout( this );
  layout->setMargin(0);

  _lineEdit = new QLineEdit(this);
  layout->addWidget(_lineEdit);

  connect(_lineEdit, SIGNAL(textChanged(const QString &)),
          this, SIGNAL(fileNameChanged(const QString &)));

  _button = new QPushButton("...", this);
  _button->setFixedWidth(_button->fontMetrics().width(" ... "));
  layout->addWidget(_button);

  connect(_button, SIGNAL(clicked()), this, SLOT(chooseFile()));
  setFocusProxy(_lineEdit);
}

// Destructor
////////////////////////////////////////////////////////////////////////////////
t_selWin::~t_selWin() {
}

// 
////////////////////////////////////////////////////////////////////////////////
void t_selWin::setFileName(const QString& fileName) {
  _lineEdit->setText(fileName);
}

// 
////////////////////////////////////////////////////////////////////////////////
QString t_selWin::fileName() const {
  return _lineEdit->text();
}

// 
////////////////////////////////////////////////////////////////////////////////
void t_selWin::chooseFile() {
  QString fileName;
  if      (mode() == File) {
    fileName = QFileDialog::getOpenFileName(this);
  }
  else if (mode() == Files) {
    QStringList fileNames = QFileDialog::getOpenFileNames(this);
    fileName = fileNames.join(",");
  }
  else {
    fileName = QFileDialog::getExistingDirectory(this);
  }

  if (!fileName.isEmpty()) {
    _lineEdit->setText(fileName);
    emit fileNameChanged(fileName);
  }
}
