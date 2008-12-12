
# Switch to debug configuration
# -----------------------------
CONFIG += release
###CONFIG += debug

DEFINES += NO_RTCM3_MAIN 
###DEFINES += DEBUG_RTCM2_2021

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

HEADERS = bnchelp.html bncgetthread.h    bncwindow.h   bnctabledlg.h  \
          bnccaster.h bncrinex.h bncapp.h bncutils.h   bnchlpdlg.h    \
          bncconst.h bnchtml.h bnctableitem.h bnczerodecoder.h        \
          RTCM/GPSDecoder.h RTCM/RTCM2.h RTCM/RTCM2Decoder.h          \
          RTCM/RTCM2_2021.h RTCM/rtcm_utils.h                         \
          RTCM3/RTCM3Decoder.h RTCM3/rtcm3torinex.h                   \
          RTCM3/RTCM3coDecoder.h RTCM3/clock_orbit_rtcm.h             \
          RTCM3/ephemeris.h RTCM3/timeutils.h                         \
          RTIGS/RTIGSDecoder.h RTIGS/rtigs_records.h                  \
          RTIGS/cgps_transform.h RTIGS/rtstruct.h RTIGS/rtacp.h RTIGS/gpswro.h

SOURCES = bncmain.cpp bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp \
          bnccaster.cpp bncrinex.cpp bncapp.cpp bncutils.cpp          \
          bncconst.cpp bnchtml.cpp bnchlpdlg.cpp bnctableitem.cpp     \
          bnczerodecoder.cpp                                          \
          RTCM/RTCM2.cpp RTCM/RTCM2Decoder.cpp                        \
          RTCM/RTCM2_2021.cpp RTCM/rtcm_utils.cpp                     \
          RTCM3/RTCM3Decoder.cpp RTCM3/rtcm3torinex.c                 \
          RTCM3/RTCM3coDecoder.cpp RTCM3/clock_orbit_rtcm.c           \
          RTCM3/ephemeris.cpp RTCM3/timeutils.cpp                     \
          RTIGS/RTIGSDecoder.cpp RTIGS/cgps_transform.cpp

RC_FILE = bnc.rc

QT += network

