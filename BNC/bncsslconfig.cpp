/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncSslConfig
 *
 * Purpose:    Singleton Class that inherits QSslConfiguration class
 *
 * Author:     L. Mervart
 *
 * Created:    22-Aug-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bncsslconfig.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSslConfig::bncSslConfig() {
  QList<QSslCertificate> caCerts = 
    QSslCertificate::fromPath("/home/mervart/certs/bkg.crt");
  std::cout << "caCerts: " << caCerts.size() << std::endl;
  this->setCaCertificates(caCerts);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSslConfig::~bncSslConfig() {
}

