
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
t_keyword::t_keyword(QString line, QTextStream& inStream) {

  _ok     = false;
  _widget = 0;  // do not delete (it is owned by layout)

  int numVal = 0;
  QTextStream in(line.toAscii(), QIODevice::ReadOnly);
  in >> _name >> numVal;

  if (!_name.isEmpty()) {
    _ok = true;

    if      (numVal == 1) {
      _values.append(in.readLine().trimmed());
    }
    else if (numVal > 1) {
      for (int ii = 0; ii < numVal; ii++) {
        _values.append(inStream.readLine().trimmed());
      }
    }

    while (inStream.status() == QTextStream::Ok && !inStream.atEnd()) {
      line = inStream.readLine().trimmed();
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
    _values.replaceInStrings(QRegExp("^\\s*\"|\"\\s*$"), QString());
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
    if (_values.size() && _values[0] == "1") {
      chBox->setChecked(true);
    }
    _widget = chBox;
  }
  else if (widgetType == "combobox") {
    QComboBox* cmbBox = new QComboBox();
    cmbBox->addItems(_desc.value("cards").split(QRegExp("\\s"), QString::SkipEmptyParts));
    if (_values.size()) {
      int index = cmbBox->findText(_values[0]);
      if (index != -1) {
        cmbBox->setCurrentIndex(index);
      }
    }
    _widget = cmbBox;
  }
  else if (widgetType == "lineedit") {
    t_lineEdit* lineEdit = new t_lineEdit();
    if (_values.size()) {
      lineEdit->setText(_values[0]);
    }
    _widget = lineEdit;
  }
  else if (widgetType == "radiobutton") {
    _widget = new QRadioButton();
  }
  else if (widgetType == "selwin") {
    _widget = new t_selWin();
  }
  else if (widgetType == "spinbox") {
    _widget = new QSpinBox();
  }
  else if (widgetType == "uniline") {
    _widget = new t_uniLine(fldMask, this);
  }

  return _widget;
}
