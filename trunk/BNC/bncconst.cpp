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

#include "bncconst.h"

const double t_CST::c       = 299792458.0;
const double t_CST::freq1   = 1575420000.0;
const double t_CST::freq2   = 1227600000.0;
const double t_CST::lambda1 = c / freq1;
const double t_CST::lambda2 = c / freq2;
const double t_CST::omega   = 7292115.1467e-11;
const double t_CST::aell    = 6378137.000;
const double t_CST::fInv    = 298.2572236;
