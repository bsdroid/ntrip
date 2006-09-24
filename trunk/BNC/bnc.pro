
CONFIG += release

DEFINES += NO_RTCM3_MAIN

RESOURCES += bnc.qrc

unix:QMAKE_CFLAGS_RELEASE   -= -O2
unix:QMAKE_CXXFLAGS_RELEASE -= -O2

win32:QMAKE_LFLAGS -= -mthreads

HEADERS = bnchelp.html bncgetthread.h    bncwindow.h   bnctabledlg.h  \
          bnccaster.h bncrinex.h bncapp.h bncutils.h   bnchlpdlg.h    \
          bncconst.h bnchtml.h                                        \
          RTCM/format.h RTCM/GPSDecoder.h RTCM/m_data.h RTCM/RTCM.h   \
          RTCM3/rtcm3.h RTCM3/rtcm3torinex.h                          \
          RTIGS/rtigs.h RTIGS/cgps_transform.h RTIGS/rtacp.h          \
          RTIGS/rtigs_records.h  RTIGS/rtstruct.h


SOURCES = bncmain.cpp bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp \
          bnccaster.cpp bncrinex.cpp bncapp.cpp bncutils.cpp          \
          bncconst.cpp bnchtml.cpp bnchlpdlg.cpp                      \
          RTCM/m_date.cpp RTCM/RTCM.cpp                               \
          RTCM3/rtcm3.cpp RTCM3/rtcm3torinex.cpp                      \
          RTIGS/rtigs.cpp RTIGS/cgps_transform.cpp

QT += network

