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
 * Class:      t_corrFile
 *
 * Purpose:    Reads DGPS Correction File
 *
 * Author:     L. Mervart
 *
 * Created:    12-Feb-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "corrfile.h"
#include "bncutils.h"
#include "bncephuser.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_corrFile::t_corrFile(QString fileName) {
  expandEnvVar(fileName);
  _file   = new QFile(fileName);
  _file->open(QIODevice::ReadOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_corrFile::~t_corrFile() {
  delete _stream;
  delete _file;
}

// Read till a given time
////////////////////////////////////////////////////////////////////////////
void t_corrFile::syncRead(const bncTime& tt) {

  if (stopRead(tt)) {
    return;
  }

  QList<QString> corrs;

  if (!_lastLine.isEmpty()) {
    corrs << _lastLine;
  }

  while (_stream->status() == QTextStream::Ok && !_stream->atEnd()) {
    QString line = _stream->readLine();
    if (line.isEmpty() || line[0] == '!') {
      continue;
    }
    _lastLine = line;

    if (stopRead(tt)) {
      if (corrs.size()) {

        QListIterator<QString> it(corrs);
        while (it.hasNext()) {
          const QString& cLine = it.next();
          t_corr* corr = new t_corr();
          corr->readLine(cLine);
          if (corr->tRao.valid()) {
            _corrIODs[corr->prn] = corr->iod;
          }
        }

        emit newCorrections(corrs);
      }
      return;
    }
    else {
      corrs << _lastLine;
    }
  }
}

// Read till a given time
////////////////////////////////////////////////////////////////////////////
bool t_corrFile::stopRead(const bncTime& tt) {

  if (_lastLine.isEmpty()) {
    return false;
  }

  QTextStream in(_lastLine.toAscii(), QIODevice::ReadOnly);
  int    messageType, updateInterval, GPSweek;
  double GPSweeks;
  in >> messageType >> updateInterval >> GPSweek >> GPSweeks;

  bncTime tNew(GPSweek, GPSweeks);

  if (tNew > tt) {
    return true;
  }    
  else {
    return false;
  }
}
