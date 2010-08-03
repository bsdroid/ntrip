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
 * Class:      bncRawFile
 *
 * Purpose:    This class stores/reads BNC raw file
 *
 * Author:     L. Mervart
 *
 * Created:    23-Aug-2010
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bncrawfile.h" 
#include "bncapp.h"
#include "bncutils.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncRawFile::bncRawFile(const QByteArray& fileName, const QByteArray& format,
                       inpOutFlag ioFlg) {
  _fileName   = fileName;
  _format     = format;
  _staID      = fileName.mid(fileName.lastIndexOf(QDir::separator())+1,5);  
  _inpFile    = 0;
  _outFile    = 0;
  _version    = 0;

  // Initialize for Input
  // --------------------
  if (ioFlg == input) {
    _inpFile = new QFile(_fileName);
    _inpFile->open(QIODevice::ReadOnly);
    QString     line = _inpFile->readLine();
    QStringList lst  = line.split(' ');
    _version = lst.value(0).toInt();

    line = _inpFile->readLine();
    bncApp* app = (bncApp*) qApp;
    app->_currentDateAndTimeGPS = 
                   new QDateTime(QDateTime::fromString(line, Qt::ISODate));
  }

  // Initialize for Output
  // ---------------------
  else {
    _outFile = new QFile(_fileName);
    _outFile->open(QIODevice::WriteOnly);
    QByteArray header = "1 Version of BNC raw file\n" +
    	                currentDateAndTimeGPS().toString(Qt::ISODate).toAscii();
    _outFile->write(header);
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncRawFile::~bncRawFile() {
  delete _inpFile;
  delete _outFile;
}

// Raw Output
////////////////////////////////////////////////////////////////////////////
void bncRawFile::writeRawData(const QByteArray& data, const QByteArray& staID,
                              const QByteArray& format) {
  if (_outFile) {
    QString chunkHeader = 
      QString("\n%1 %2 %3\n").arg(QString(staID)).arg(QString(format)).arg(data.size());
    _outFile->write(chunkHeader.toAscii());
    _outFile->write(data);
    _outFile->flush();
  }
}


// Raw Input
////////////////////////////////////////////////////////////////////////////
QByteArray bncRawFile::readChunk() {

  QByteArray data;

  if (_inpFile) {
    QString     line = _inpFile->readLine();
    QStringList lst  = line.split(' ');
    
    QByteArray staID_current  = lst.value(0).toAscii();
    QByteArray format_current = lst.value(1).toAscii();
    int        nBytes         = lst.value(2).toInt();

    data = _inpFile->read(nBytes);

    _inpFile->read(1); // read '\n' character
  }

  return data;
}

