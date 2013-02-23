
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
using namespace RTGUI;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_keyword::t_keyword(QString line, QTextStream& inStream) {

  _ok     = false;
  _type   = e_type_max;
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
          if (descKey == "widget") {
            if      (descVal == "checkbox") {
              _type = checkbox;
            }
            else if (descVal == "combobox") {
              _type = combobox;
            }
            else if (descVal == "lineedit") {
              _type = lineedit;
            }
            else if (descVal == "radiobutton") {
              _type = radiobutton;
            }
            else if (descVal == "selwin") {
              _type = selwin;
            }
            else if (descVal == "spinbox") {
              _type = spinbox;
            }
            else if (descVal == "uniline") {
              _type = uniline;
            }
          }
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

  if      (_type == checkbox) {
    _widget = new QCheckBox(); 
  }
  else if (_type == combobox) {
    _widget = new QComboBox();
  }
  else if (_type == lineedit) {
    t_lineEdit* lineEdit = new t_lineEdit();
    if (_values.size()) {
      lineEdit->setText(_values[0]);
    }
    _widget = lineEdit;
  }
  else if (_type == radiobutton) {
    _widget = new QRadioButton();
  }
  else if (_type == selwin) {
    _widget = new t_selWin();
  }
  else if (_type == spinbox) {
    _widget = new QSpinBox();
  }
  else if (_type == uniline) {
    _widget = new t_uniLine(fldMask, this);
  }

  return _widget;
}
