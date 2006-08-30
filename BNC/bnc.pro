
HEADERS =             bncgetthread.h    bncwindow.h   bnctabledlg.h   \
          bnccaster.h bncrinex.h bncapp.h bncutils.h                  \
          RTCM/format.h RTCM/GPSDecoder.h RTCM/m_data.h RTCM/RTCM.h   \
          RTCM3/rtcm3.h RTCM3/rtcm3torinex.h                          \
          RTIGS/rtigs.h RTIGS/cgps_transform.h RTIGS/rtacp.h          \
          RTIGS/rtigs_records.h  RTIGS/rtstruct.h


SOURCES = bncmain.cpp bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp \
          bnccaster.cpp bncrinex.cpp bncapp.cpp bncutils.cpp          \
          RTCM/m_date.cpp RTCM/RTCM.cpp                               \
          RTCM3/rtcm3.cpp RTCM3/rtcm3torinex.cpp                      \
          RTIGS/rtigs.cpp RTIGS/cgps_transform.cpp

QT += network

