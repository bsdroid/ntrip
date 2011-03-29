// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncTableDlg
 *
 * Purpose:    Displays the source table, allows mountpoints selection
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bnctabledlg.h"
#include "bncgetthread.h"
#include "bncnetqueryv1.h"
#include "bncsettings.h"
#include "bncmap.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncTableDlg::bncTableDlg(QWidget* parent) : QDialog(parent) {

  setMinimumSize(600,400);
  setWindowTitle(tr("Add Streams from Caster"));

  QVBoxLayout* mainLayout = new QVBoxLayout(this);

  int ww = QFontMetrics(font()).width('w');

  _casterPortLineEdit = new QLineEdit();
  _casterPortLineEdit->setMaximumWidth(9*ww);

  _casterUserLineEdit = new QLineEdit();
  _casterUserLineEdit->setMaximumWidth(9*ww);

  _casterPasswordLineEdit = new QLineEdit();
  _casterPasswordLineEdit->setMaximumWidth(9*ww);
  _casterPasswordLineEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);

  _casterHostComboBox = new QComboBox();
  _casterHostComboBox->setMaxCount(10);
  _casterHostComboBox->setDuplicatesEnabled(false);
  _casterHostComboBox->setEditable(true);
  _casterHostComboBox->setMinimumWidth(20*ww);
  _casterHostComboBox->setMaximumWidth(20*ww);
  connect(_casterHostComboBox, SIGNAL(currentIndexChanged(const QString&)),
          this, SLOT(slotCasterHostChanged(const QString&)));
  bncSettings settings;
  settings.remove("casterHostList");
  settings.remove("casterHost");
  settings.remove("casterPort");
  settings.remove("casterUser");
  settings.remove("casterPassword");
  QStringList casterUrlList = settings.value("casterUrlList").toStringList();
  for (int ii = 0; ii < casterUrlList.count(); ii++) {
    QUrl url(casterUrlList[ii]);
    _casterHostComboBox->addItem(url.host());
  }

  _ntripVersionComboBox = new QComboBox();
  _ntripVersionComboBox->addItems(QString("1,2,R,U").split(","));
  int kk = _ntripVersionComboBox->findText(settings.value("ntripVersion").toString());
  if (kk != -1) {
    _ntripVersionComboBox->setCurrentIndex(kk);
  }
  _ntripVersionComboBox->setMaximumWidth(5*ww);

  _buttonCasterTable = new QPushButton(tr("Show"), this);
  connect(_buttonCasterTable, SIGNAL(clicked()), this, SLOT(slotCasterTable()));
  _buttonCasterTable->setMaximumWidth(5*ww);

  // WhatsThis
  // ---------
  _casterUserLineEdit->setWhatsThis(tr("<p>Access to some streams on NTRIP broadcasters may be restricted. You'll need to enter a valid 'User ID' and 'Password' for access to these protected streams.</p><p>Accounts are usually provided per NTRIP broadcaster through a registration process. Register through <u>http://igs.bkg.bund.de/index_ntrip_reg.htm</u> for access to protected streams on <u>www.euref-ip.net</u> and <u>www.igs-ip.net</u>.</p>"));
  _casterHostComboBox->setWhatsThis(tr("<p>Enter the NTRIP broadcaster hostname or IP number.</p><p>Note that EUREF and IGS operate NTRIP broadcasters at <u>http://www.euref-ip.net/home</u> and <u>http://www.igs-ip.net/home</u>.</p>"));
  _casterPortLineEdit->setWhatsThis(tr("Enter the NTRIP broadcaster port number."));
  _casterPasswordLineEdit->setWhatsThis(tr("Access to some streams on NTRIP broadcasters may be restricted. You'll need to enter a valid 'Password' for access to these protected streams."));

  QGridLayout* editLayout = new QGridLayout;
  editLayout->addWidget(new QLabel(tr("Caster host")),    0, 0);
  editLayout->addWidget(_casterHostComboBox,              0, 1);
  editLayout->addWidget(new QLabel(tr("  Caster port")),  0, 2, Qt::AlignRight);
  editLayout->addWidget(_casterPortLineEdit,              0, 3);
  editLayout->addWidget(new QLabel(tr("Casters table")),  0, 4, Qt::AlignRight);
  editLayout->addWidget(_buttonCasterTable,               0, 5);
  editLayout->addWidget(new QLabel(tr("User")),           1, 0, Qt::AlignRight);
  editLayout->addWidget(_casterUserLineEdit,              1, 1);
  editLayout->addWidget(new QLabel(tr("Password")),       1, 2, Qt::AlignRight);
  editLayout->addWidget(_casterPasswordLineEdit,          1, 3);
  editLayout->addWidget(new QLabel(tr("  NTRIP Version")),1, 4, Qt::AlignRight);
  editLayout->addWidget(_ntripVersionComboBox,            1, 5);

  mainLayout->addLayout(editLayout);

  _buttonCasterTable->setWhatsThis(tr("<p>Hit 'Show' for a table of known NTRIP broadcaster installations as maintained at <u>www.rtcm-ntrip.org/home.</u></p><p>A window opens which allows to select a broadcaster for stream retrieval.</p>"));
  _ntripVersionComboBox->setWhatsThis(tr("<p>Select the NTRIP transport protocol version you want to use. Implemented options are:<br>&nbsp; 1:&nbsp; NTRIP version 1, TCP/IP<br>&nbsp; 2:&nbsp; NTRIP version 2, TCP/IP mode<br>&nbsp; R:&nbsp; NTRIP Version 2, RTSP/RTP mode<br>&nbsp; U:&nbsp; NTRIP Version 2, UDP mode<br>Select option '1' if you are not sure whether the NTRIP broadcaster supports NTRIP version 2.</p><p>Note that RTSP/RTP (option 'R') and UDP (option 'U') are not accepted by proxies and sometimes not supported by mobile Internet Service Providers.</p>"));

  _table = new QTableWidget(this);
  _table->setWhatsThis(tr("<p>Use the 'Get Table' button to download the sourcetable from the selected NTRIP broadcaster. Select the desired streams line by line, using +Shift and +Ctrl when necessary. Hit 'OK' to return to the main window.</p><p>Pay attention to data field 'format'. Keep in mind that BNC can only decode and convert streams that come in RTCM Version 2.x, RTCM Version 3.x, or RTIGS format. See data field 'format-details' for available message types and their repetition rates in brackets.</p><p>The content of data field 'nmea' tells you whether a stream comes from a virtual reference station (VRS).</p>"));
  connect(_table, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotSelectionChanged()));
  mainLayout->addWidget(_table);

  _buttonWhatsThis = new QPushButton(tr("Help=Shift+F1"), this);
  connect(_buttonWhatsThis, SIGNAL(clicked()), this, SLOT(slotWhatsThis()));

  _buttonGet = new QPushButton(tr("Get table"), this);
  _buttonGet->setDefault(true);
  connect(_buttonGet, SIGNAL(clicked()), this, SLOT(slotGetTable()));
 
  _buttonMap = new QPushButton(tr("Map"), this);
  _buttonMap->setEnabled(false);
  connect(_buttonMap, SIGNAL(clicked()), this, SLOT(slotShowMap()));
 
  _buttonClose = new QPushButton(tr("Close"), this);
  connect(_buttonClose, SIGNAL(clicked()), this, SLOT(close()));

  _buttonSelect = new QPushButton(tr("Select"), this);
  connect(_buttonSelect, SIGNAL(clicked()), this, SLOT(select()));

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(_buttonWhatsThis);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(_buttonMap);
  buttonLayout->addWidget(_buttonGet);
  buttonLayout->addWidget(_buttonSelect);
  buttonLayout->addWidget(_buttonClose);

  mainLayout->addLayout(buttonLayout);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncTableDlg::~bncTableDlg() {
  delete _casterHostComboBox;
  delete _casterPortLineEdit;
  delete _casterUserLineEdit;
  delete _casterPasswordLineEdit;
  delete _ntripVersionComboBox;
  delete _buttonGet;
  delete _buttonClose;
  delete _buttonSelect;
  delete _buttonWhatsThis;
  delete _buttonCasterTable;
  delete _table;
}

// Read full caster table (static)
////////////////////////////////////////////////////////////////////////////
t_irc bncTableDlg::getFullTable(const QString& casterHost, 
                                int casterPort, QStringList& allLines, 
                                bool alwaysRead) {

  static QMutex mutex;
  static QMap<QString, QStringList> allTables;

  QMutexLocker locker(&mutex);

  if (!alwaysRead && allTables.find(casterHost) != allTables.end()) {
    allLines = allTables.find(casterHost).value();
    return success;
  }

  allLines.clear();

  QUrl url;
  url.setHost(casterHost);
  url.setPort(casterPort);
  url.setScheme("http");
  url.setPath("/");

  bncNetQueryV1 query;
  QByteArray outData;
  query.waitForRequestResult(url, outData);
  if (query.status() == bncNetQuery::finished) {
    QTextStream in(outData);
    QString line = in.readLine();
    while ( !line.isNull() ) {
      allLines.append(line);
      line = in.readLine();
    } 
    allTables.insert(casterHost, allLines);
    return success;
  }
  else {
    return failure;
  }
}

// Read Table from Caster
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotGetTable() {

  _buttonGet->setEnabled(false);
  _buttonMap->setEnabled(true);
  _buttonCasterTable->setEnabled(false); 

  if ( getFullTable(_casterHostComboBox->currentText(),
                    _casterPortLineEdit->text().toInt(),
                    _allLines, true) != success ) {
    QMessageBox::warning(0, "BNC", "Cannot retrieve table of data");
    _buttonGet->setEnabled(true);
    return;
  }
  
  static const QStringList labels = QString("mountpoint,identifier,format,"
    "format-details,carrier,system,network,country,lat,long,"
    "nmea,solution,generator,compress.,auth.,fee,bitrate,"
    "misc").split(",");

  QStringList lines;
  QStringListIterator it(_allLines);
  while (it.hasNext()) {
    QString line = it.next();
    if (line.indexOf("STR") == 0) {
      QStringList hlp = line.split(";");
      if (hlp.size() > labels.size()) {
        lines.push_back(line);
      }
    }
  }

  if (lines.size() > 0) {
    _table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setColumnCount(labels.size());
    _table->setRowCount(lines.size());
    for (int nRow = 0; nRow < lines.size(); nRow++) {
      QStringList columns = lines[nRow].split(";");
      for (int ic = 1; ic < columns.size(); ic++) {
        if (ic == 11) { 
          if (columns[ic] == "0") { 
            columns[ic] = "no"; 
          } else { 
            columns[ic] = "yes"; 
          }
        }
        QTableWidgetItem* item = new QTableWidgetItem(columns[ic]);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        _table->setItem(nRow, ic-1, item);
      }
    }
    _table->setHorizontalHeaderLabels(labels);
    _table->setSortingEnabled(true);
    int ww = QFontMetrics(this->font()).width('w');
    _table->horizontalHeader()->resizeSection( 0,10*ww);
    _table->horizontalHeader()->resizeSection( 1,10*ww);
    _table->horizontalHeader()->resizeSection( 2, 8*ww);
    _table->horizontalHeader()->resizeSection( 3,22*ww);
    _table->horizontalHeader()->resizeSection( 4, 5*ww);
    _table->horizontalHeader()->resizeSection( 5, 8*ww);
    _table->horizontalHeader()->resizeSection( 6, 8*ww);
    _table->horizontalHeader()->resizeSection( 7, 7*ww);
    _table->horizontalHeader()->resizeSection( 8, 6*ww);
    _table->horizontalHeader()->resizeSection( 9, 6*ww);
    _table->horizontalHeader()->resizeSection(10, 6*ww);
    _table->horizontalHeader()->resizeSection(11, 6*ww);
    _table->horizontalHeader()->resizeSection(12,15*ww);
    _table->horizontalHeader()->resizeSection(13, 8*ww);
    _table->horizontalHeader()->resizeSection(14, 5*ww);
    _table->horizontalHeader()->resizeSection(15, 5*ww);
    _table->horizontalHeader()->resizeSection(16, 7*ww);
    _table->horizontalHeader()->resizeSection(17,15*ww);
    _table->sortItems(0);
  }
}

// Show world map
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotShowMap() {

  bncMap* winMap = new bncMap(this);
  winMap->setGeometry( x(), int(y()+height()*1.3), 880, 440 );

  connect(this, SIGNAL(newPoint(QPointF, QString, QPen)),
	  winMap, SLOT(slotNewPoint(QPointF, QString, QPen)));
      
  connect(this, SIGNAL(fitMap()),
	  winMap, SLOT(slotFitMap() ));
      
  _buttonMap->setEnabled(false);
  showSourceTable();
  winMap->exec();
  _buttonMap->setEnabled(true);

  disconnect(this, SIGNAL(newPoint(QPointF, QString, QPen)),
	     winMap, SLOT(slotNewPoint(QPointF, QString, QPen)));
   
  disconnect(this, SIGNAL(fitMap()),
 	     winMap, SLOT(slotFitMap() ));
   
  delete winMap;
}

// Show world map
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::showSourceTable() {

  for( int i = 0; i < _allLines.size(); i++ ){
	
    if( _allLines.at(i).startsWith("STR") == true ){
	     
       QStringList tmp = _allLines.at(i).split(';');
       if( tmp.size() > 0 ){
		  
	 QPointF point;
    	         point.setY( tmp.at(9).toDouble() );
                 point.setX( tmp.at(10).toDouble() );

	 QString site = tmp.at(1);
	         site.resize(4);

         emit newPoint(point, site, QPen(QBrush(QColor(0,0,255,200)), 1.5) );
       }
     }
   }
   emit fitMap();
}


// Select slot
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::select() {

  bncSettings settings;
  settings.setValue("ntripVersion", _ntripVersionComboBox->currentText());
  QUrl url;
  url.setScheme("http");
  url.setHost(_casterHostComboBox->currentText());
  url.setPort(_casterPortLineEdit->text().toInt());
  url.setUserName(QUrl::toPercentEncoding(_casterUserLineEdit->text()));
  url.setPassword(QUrl::toPercentEncoding(_casterPasswordLineEdit->text()));
  addUrl(url);

  QStringList* mountPoints = new QStringList;
  if (_table) {
    for (int ir = 0; ir < _table->rowCount(); ir++) {
      QTableWidgetItem* item   = _table->item(ir,0);
      QString             site = _table->item(ir,0)->text();
      QString           format = _table->item(ir,2)->text();
      QString         latitude = _table->item(ir,8)->text();
      QString        longitude = _table->item(ir,9)->text();
      QString             nmea = _table->item(ir,10)->text();
      QString     ntripVersion = _ntripVersionComboBox->currentText();
      format.replace(" ", "_");
      if (_table->isItemSelected(item)) {
        url.setPath(item->text());
        mountPoints->push_back(url.toString() + " " + format + " " + latitude
                        + " " + longitude + " " + nmea + " " + ntripVersion);
	 
        site.resize(4);
	emit newPoint(QPointF(longitude.toDouble(),latitude.toDouble()), site,
		      QPen(QBrush(QColor(255,0,0,200)), 3) );
      }
    }
  }
  emit newMountPoints(mountPoints);
}

// User changed the selection of mountPoints
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotSelectionChanged() {
  if (_table->selectedItems().isEmpty()) {
  }
}

// Whats This Help
void bncTableDlg::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}

// Slot caster table
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotCasterTable() {
        
  _buttonCasterTable->setEnabled(false);
  _casterHostComboBox->setEnabled(false);
  _casterPortLineEdit->setEnabled(false);
  _casterUserLineEdit->setEnabled(false);
  _casterPasswordLineEdit->setEnabled(false);
  _ntripVersionComboBox->setEnabled(false);
  _buttonWhatsThis->setEnabled(false);
  _buttonGet->setEnabled(false);
  _buttonClose->setEnabled(false);
  _buttonSelect->setEnabled(false);

  bncCasterTableDlg* dlg = new bncCasterTableDlg(this);
  dlg->move(this->pos().x()+50, this->pos().y()+50);
  connect(dlg, SIGNAL(newCaster(QString, QString)),
          this, SLOT(slotNewCaster(QString, QString)));
  dlg->exec();
  delete dlg;

  _buttonCasterTable->setEnabled(true);
  _casterHostComboBox->setEnabled(true);
  _casterPortLineEdit->setEnabled(true);
  _casterUserLineEdit->setEnabled(true);
  _casterPasswordLineEdit->setEnabled(true);
  _ntripVersionComboBox->setEnabled(true);
  _buttonWhatsThis->setEnabled(true);
  _buttonGet->setEnabled(true);
  _buttonClose->setEnabled(true);
  _buttonSelect->setEnabled(true);

}

// New caster selected
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotNewCaster(QString newCasterHost, QString newCasterPort) {

  _casterHostComboBox->insertItem(0, newCasterHost);
  _casterHostComboBox->setCurrentIndex(0);
  _casterUserLineEdit->setText("");
  _casterPasswordLineEdit->setText("");
  _casterPortLineEdit->setText(newCasterPort);

  QUrl url;
  url.setScheme("http");
  url.setHost(newCasterHost);
  url.setPort(newCasterPort.toInt());
  addUrl(url);
  
  _casterHostComboBox->setCurrentIndex(0);
}

// New caster selected
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::addUrl(const QUrl& url) {
  bncSettings settings;
  QStringList oldUrlList = settings.value("casterUrlList").toStringList();
  QStringList newUrlList;
  newUrlList << url.toString();
  for (int ii = 0; ii < oldUrlList.count(); ii++) {
    QUrl oldUrl(oldUrlList[ii]);
    if (url.host() != oldUrl.host()) {
      newUrlList << oldUrl.toString();
    }
  }
  settings.setValue("casterUrlList", newUrlList);
  settings.sync();
}

// New caster selected in combobox
////////////////////////////////////////////////////////////////////////////
void bncTableDlg::slotCasterHostChanged(const QString& newHost) {
  bncSettings settings;
  QStringList casterUrlList = settings.value("casterUrlList").toStringList();
  for (int ii = 0; ii < casterUrlList.count(); ii++) {
    QUrl url(casterUrlList[ii]);
    if (url.host() == newHost) {
      _casterUserLineEdit->setText(
                QUrl::fromPercentEncoding(url.userName().toAscii()));
      _casterPasswordLineEdit->setText(
                QUrl::fromPercentEncoding(url.password().toAscii()));
      if (url.port() > 0) {
        _casterPortLineEdit->setText(QString("%1").arg(url.port()));
      }
      else {
        _casterPortLineEdit->setText("");
      }
    }
  }
}

// Caster table
////////////////////////////////////////////////////////////////////////////
bncCasterTableDlg::bncCasterTableDlg(QWidget* parent) : 
   QDialog(parent) {

  static const QStringList labels = QString("host,port,identifier,operator,nmea,country,lat,long,link").split(",");

  _casterTable = new QTableWidget(this);

  QUrl url;
  url.setHost("www.rtcm-ntrip.org");
  url.setPort(2101);
  url.setScheme("http");
  url.setPath("/");

  bncNetQueryV1 query;
  QByteArray outData;
  query.waitForRequestResult(url, outData);

  QStringList lines;
  if (query.status() == bncNetQuery::finished) {
    QTextStream in(outData);
    QString line = in.readLine();
    while ( !line.isNull() ) {
      line = in.readLine();
      if (line.indexOf("CAS") == 0) {
        QStringList hlp = line.split(";");
        if (hlp.size() > labels.size()) {
          lines.push_back(line);
        }
      }
    }
  }

  if (lines.size() > 0) {
    _casterTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _casterTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList hlp = lines[0].split(";");
    _casterTable->setColumnCount(labels.size());
    _casterTable->setRowCount(lines.size());

    for (int nRow = 0; nRow < lines.size(); nRow++) {
      QStringList columns = lines[nRow].split(";");
      for (int ic = 1; ic < columns.size(); ic++) {
         if (ic == 5) { 
           if (columns[ic] == "0") { 
             columns[ic] = "no"; 
           } else { 
             columns[ic] = "yes"; 
           }
         }
         QTableWidgetItem* item = new QTableWidgetItem(columns[ic]);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        _casterTable->setItem(nRow, ic-1, item);
      }
    }
  } 
  _casterTable->setHorizontalHeaderLabels(labels);
  _casterTable->setSortingEnabled(true);
  _casterTable->sortItems(0);
   int ww = QFontMetrics(this->font()).width('w');
  _casterTable->horizontalHeader()->resizeSection(0,15*ww);
  _casterTable->horizontalHeader()->resizeSection(1, 5*ww);
  _casterTable->horizontalHeader()->resizeSection(2,15*ww);
  _casterTable->horizontalHeader()->resizeSection(3,15*ww);
  _casterTable->horizontalHeader()->resizeSection(4, 5*ww);
  _casterTable->horizontalHeader()->resizeSection(5, 7*ww);
  _casterTable->horizontalHeader()->resizeSection(6, 7*ww);
  _casterTable->horizontalHeader()->resizeSection(7, 7*ww);
  _casterTable->horizontalHeader()->resizeSection(8,15*ww);

  _closeButton = new QPushButton("Cancel");
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));
  _closeButton->setMinimumWidth(8*ww);
  _closeButton->setMaximumWidth(8*ww);

  _okButton = new QPushButton(tr("OK"), this);
  connect(_okButton, SIGNAL(clicked()), this, SLOT(slotAcceptCasterTable()));
  _okButton->setMinimumWidth(8*ww);
  _okButton->setMaximumWidth(8*ww);

  _whatsThisButton = new QPushButton(tr("Help=Shift+F1"), this);
  connect(_whatsThisButton, SIGNAL(clicked()), this, SLOT(slotWhatsThis()));
  _whatsThisButton->setMinimumWidth(12*ww);
  _whatsThisButton->setMaximumWidth(12*ww);

  _casterTable->setWhatsThis(tr("<p>Select an NTRIP broadcaster and hit 'OK'.</p><p>See <u>http://www.rtcm-ntrip.org/home</u> for further details on known NTRIP broadcaster installations.</u>."));

  QGridLayout* dlgLayout = new QGridLayout();
  dlgLayout->addWidget(new QLabel("  List of NTRIP Broadcasters from www.rtcm-ntrip.org"), 0,0,1,3,Qt::AlignLeft);
  dlgLayout->addWidget(_casterTable,     1, 0, 1, 3);
  dlgLayout->addWidget(_whatsThisButton, 2, 0);
  dlgLayout->addWidget(_closeButton,     2, 1, Qt::AlignRight);  
  dlgLayout->addWidget(_okButton,        2, 2);

  setMinimumSize(600,400);
  setWindowTitle(tr("Select Broadcaster"));
  setLayout(dlgLayout);
  resize(68*ww, 50*ww);
  show();
}

// Caster table destructor
////////////////////////////////////////////////////////////////////////////
bncCasterTableDlg::~bncCasterTableDlg() {
  delete _casterTable;
  delete _okButton;
  delete _closeButton;
  delete _whatsThisButton;
}

// Caster table what's this
////////////////////////////////////////////////////////////////////////////
void bncCasterTableDlg:: slotWhatsThis() {
  QWhatsThis::enterWhatsThisMode();
}

// Accept caster table
////////////////////////////////////////////////////////////////////////////
void bncCasterTableDlg::slotAcceptCasterTable() {
  if (_casterTable) {
    for (int ir = _casterTable->rowCount() - 1; ir >= 0 ; ir--) {
      if (_casterTable->isItemSelected(_casterTable->item(ir,0))) {
        emit newCaster(_casterTable->item(ir,0)->text(), 
                       _casterTable->item(ir,1)->text());
      }
    }
  }
  QDialog::accept();
}
