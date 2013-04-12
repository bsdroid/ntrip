
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_keyword
 *
 * Purpose:    Keyword in RTNet Input File
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "keyword.h"
#include "lineedit.h"
#include "selwin.h"
#include "uniline.h"

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_keyword::t_keyword(QString line, QTextStream& inStream, QStringList& staticLines) {

  _ok     = false;
  _widget = 0;  // do not delete (it is owned by layout)

  int numVal = 0;
  QTextStream in(line.toAscii(), QIODevice::ReadOnly);
  in >> _name >> numVal;

  staticLines << _name;

  if (!_name.isEmpty()) {
    _ok = true;

    if      (numVal == 1) {
      _origValues.append(in.readLine().trimmed());
    }
    else if (numVal > 1) {
      for (int ii = 0; ii < numVal; ii++) {
        _origValues.append(inStream.readLine().trimmed());
      }
    }

    while (inStream.status() == QTextStream::Ok && !inStream.atEnd()) {
      line = inStream.readLine();
      staticLines << line;
      line = line.trimmed();
      if      (line.isEmpty()) {
        break;
      }
      else if (line[0] == '#') {
        int pos = 0;
        while (true) {
          QRegExp rx("([^#;]*)\\s+=\\s+([^#;]*)");
          pos = rx.indexIn(line, pos);
          if (pos == -1) {
            break;
          }
          else {
            pos += rx.matchedLength();
          }
          QString descKey = rx.cap(1).trimmed();
          QString descVal = rx.cap(2).trimmed();
          _desc[descKey]  = descVal;
        }
      }
    }

    // Remove leading and trailing double-quotes
    // -----------------------------------------
    _origValues.replaceInStrings(QRegExp("^\\s*\"|\"\\s*$"), QString());
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_keyword::~t_keyword() {
}

// Create Widget (it will be owned by layout)
////////////////////////////////////////////////////////////////////////////
QWidget* t_keyword::createWidget(const QString& fldMask) {

  if (_widget != 0) {
    // TODO: exception
  }

  QString widgetType = _desc.value("widget");

  if      (widgetType == "checkbox") {
    QCheckBox* chBox = new QCheckBox(); 
    if (_origValues.size() && _origValues[0] == "1") {
      chBox->setChecked(true);
    }
    _widget = chBox;
  }
  else if (widgetType == "combobox") {
    QComboBox* cmbBox = new QComboBox();
    cmbBox->addItems(_desc.value("cards").split(QRegExp("\\s"), QString::SkipEmptyParts));
    if (_origValues.size()) {
      int index = cmbBox->findText(_origValues[0]);
      if (index != -1) {
        cmbBox->setCurrentIndex(index);
      }
    }
    _widget = cmbBox;
  }
  else if (widgetType == "lineedit") {
    t_lineEdit* lineEdit = new t_lineEdit();
    if (_origValues.size()) {
      lineEdit->setText(_origValues[0]);
    }
    _widget = lineEdit;
  }
  else if (widgetType == "radiobutton") {
    QRadioButton* radButt = new QRadioButton();
    if (_origValues.size() && _origValues[0] == "1") {
      radButt->setChecked(true);
    }
    _widget = radButt;
  }
  else if (widgetType == "selwin") {
    t_selWin* selWin = new t_selWin();
    if (_origValues.size()) {
      selWin->setFileName(_origValues[0]);
    }
    _widget = selWin;
  }
  else if (widgetType == "spinbox") {
    QSpinBox* spinBox = new QSpinBox();
    QStringList rangeStr = _desc.value("range").split(QRegExp("\\s"), QString::SkipEmptyParts);
    if (rangeStr.size() >= 1) spinBox->setMinimum(rangeStr[0].toInt());
    if (rangeStr.size() >= 2) spinBox->setMaximum(rangeStr[1].toInt());
    if (rangeStr.size() >= 3) spinBox->setSingleStep(rangeStr[2].toInt());
    _widget = spinBox;
  }
  else if (widgetType == "uniline") {
    _widget = new t_uniLine(fldMask, this);
  }

  return _widget;
}

// 
////////////////////////////////////////////////////////////////////////////
QStringList t_keyword::values() const {
  return _origValues;
}

