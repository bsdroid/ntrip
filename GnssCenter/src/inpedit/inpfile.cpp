
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_inpFile
 *
 * Purpose:    RTNet Input File
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "inpfile.h" 
#include "keyword.h" 
#include "panel.h" 

using namespace std;
using namespace RTGUI;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_inpFile::t_inpFile(const QString& fileName) : QTabWidget(0) {
  _fileName = fileName;
  readFile();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_inpFile::~t_inpFile() {
  QMapIterator<QString, t_keyword*> it(_keywords); 
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_inpFile::readFile() {

  QFile file(_fileName);
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream inStream(&file);

  int iPanel = 0;

  while (inStream.status() == QTextStream::Ok && !inStream.atEnd()) {
    QString line = inStream.readLine().trimmed();

    // Skip Comments and empty Lines
    // -----------------------------
    if      (line.isEmpty() || line[0] == '!') {
      continue;
    }

    // Read Panels
    // -----------
    else if (line[0] == '#' && line.indexOf("BEGIN_PANEL") != -1) {
      t_panel* panel = new t_panel(line, inStream, &_keywords);
      if (panel->ok()) {
        ++iPanel;
        addTab(panel, QString("Panel %1").arg(iPanel));
      }
      else {
        delete panel;
      }
    }

    // Read Keywords
    // -------------
    else {
      t_keyword* keyword = new t_keyword(line, inStream);
      if (keyword->ok()) {
        _keywords[keyword->name()] = keyword;
      }
      else {
        delete keyword;
      }
    }
  }
}
