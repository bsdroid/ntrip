
# Switch to debug configuration
# -----------------------------
CONFIG -= debug
CONFIG += release

DEFINES += NO_RTCM3_MAIN 
###DEFINES += DEBUG_RTCM2_2021
unix:DEFINES  += _TTY_POSIX_
win32:DEFINES += _TTY_WIN_

RESOURCES += bnc.qrc

unix:QMAKE_CFLAGS_RELEASE   -= -O2
unix:QMAKE_CXXFLAGS_RELEASE -= -O2

# Get rid of mingwm10.dll
# -----------------------
win32:QMAKE_LFLAGS                 -= -mthreads
win32:QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
win32:QMAKE_LFLAGS_EXCEPTIONS_ON   -= -mthreads

debug:OBJECTS_DIR=.obj/debug
debug:MOC_DIR=.moc/debug
release:OBJECTS_DIR=.obj/release
release:MOC_DIR=.moc/release

# Include Path
# ------------
INCLUDEPATH = .

HEADERS = bnchelp.html bncgetthread.h    bncwindow.h   bnctabledlg.h  \
          bnccaster.h bncrinex.h bncapp.h bncutils.h   bnchlpdlg.h    \
          bncconst.h bnchtml.h bnctableitem.h bnczerodecoder.h        \
          bncnetquery.h bncnetqueryv1.h bncnetqueryv2.h               \
          bncnetqueryrtp.h bncsettings.h latencychecker.h             \
          bncipport.h bncnetqueryv0.h bncnetqueryudp.h                \ 
          bncnetqueryudp0.h bncudpport.h                              \ 
          bncserialport.h bncnetquerys.h bncfigure.h                  \ 
          bncfigurelate.h bncpppthread.h bncversion.h                 \ 
          RTCM/GPSDecoder.h RTCM/RTCM2.h RTCM/RTCM2Decoder.h          \
          RTCM/RTCM2_2021.h RTCM/rtcm_utils.h                         \
          RTCM3/RTCM3Decoder.h RTCM3/rtcm3torinex.h                   \
          RTCM3/RTCM3coDecoder.h RTCM3/clock_orbit_rtcm.h             \
          RTCM3/ephemeris.h RTCM3/timeutils.h                         \
          RTIGS/RTIGSDecoder.h RTIGS/rtigs_records.h                  \
          RTIGS/cgps_transform.h RTIGS/rtstruct.h                     \
          RTIGS/rtacp.h RTIGS/gpswro.h                                \
          GPSS/gpssDecoder.h

HEADERS       += serial/qextserialbase.h serial/qextserialport.h
unix:HEADERS  += serial/posix_qextserialport.h
win32:HEADERS += serial/win_qextserialport.h

SOURCES = bncmain.cpp bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp \
          bnccaster.cpp bncrinex.cpp bncapp.cpp bncutils.cpp          \
          bncconst.cpp bnchtml.cpp bnchlpdlg.cpp bnctableitem.cpp     \
          bnczerodecoder.cpp bncnetqueryv1.cpp bncnetqueryv2.cpp      \
          bncnetqueryrtp.cpp bncsettings.cpp latencychecker.cpp       \
          bncipport.cpp bncnetqueryv0.cpp bncnetqueryudp.cpp          \
          bncnetqueryudp0.cpp bncudpport.cpp                          \
          bncserialport.cpp bncnetquerys.cpp bncfigure.cpp            \
          bncfigurelate.cpp bncpppthread.cpp                          \
          RTCM/RTCM2.cpp RTCM/RTCM2Decoder.cpp                        \
          RTCM/RTCM2_2021.cpp RTCM/rtcm_utils.cpp                     \
          RTCM3/RTCM3Decoder.cpp RTCM3/rtcm3torinex.c                 \
          RTCM3/RTCM3coDecoder.cpp RTCM3/clock_orbit_rtcm.c           \
          RTCM3/ephemeris.cpp RTCM3/timeutils.cpp                     \
          RTIGS/RTIGSDecoder.cpp RTIGS/cgps_transform.cpp             \
          GPSS/gpssDecoder.cpp

SOURCES       += serial/qextserialbase.cpp serial/qextserialport.cpp
unix:SOURCES  += serial/posix_qextserialport.cpp
win32:SOURCES += serial/win_qextserialport.cpp

RC_FILE = bnc.rc

QT += network

