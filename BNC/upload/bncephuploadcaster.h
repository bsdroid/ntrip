#ifndef BNCEPHUPLOADCASTER_H
#define BNCEPHUPLOADCASTER_H

#include <newmat.h>
#include "bncuploadcaster.h"
#include "bncephuser.h"

class bncEphUploadCaster : public bncEphUser {
 Q_OBJECT
 public:
  bncEphUploadCaster();
  virtual ~bncEphUploadCaster();
 protected:
  virtual void ephBufferChanged();
 private:
  bncUploadCaster* _ephUploadCaster;
};

#endif
