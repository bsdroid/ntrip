
HEADERS =             bncgetthread.h    bncwindow.h   bnctabledlg.h   \
          bnccaster.h \
          RTCM/format.h RTCM/GPSDecoder.h RTCM/m_data.h RTCM/RTCM.h   \
          RTIGS/rtigs.h  RTIGS/cgps_transform.h  RTIGS/rtacp.h        \
          RTIGS/rtigs_records.h  RTIGS/rtstruct.h


SOURCES = bncmain.cpp bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp \
          bnccaster.cpp \
          RTCM/m_date.cpp RTCM/RTCM.cpp \
          RTIGS/rtigs.cpp RTIGS/cgps_transform.cpp

QT += network

