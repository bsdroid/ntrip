
# Switch to debug configuration
# -----------------------------
CONFIG -= release
CONFIG += debug

DEFINES += NO_RTCM3_MAIN 

# Get rid of mingwm10.dll
# -----------------------
win32:QMAKE_LFLAGS                 -= -mthreads
win32:QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
win32:QMAKE_LFLAGS_EXCEPTIONS_ON   -= -mthreads

debug:OBJECTS_DIR=.obj/debug
debug:MOC_DIR=.moc/debug
release:OBJECTS_DIR=.obj/release
release:MOC_DIR=.moc/release

HEADERS =                bncnetrequest.h   bncutils.h   bncrinex.h    \
          RTCM/GPSDecoder.h RTCM/RTCM2.h RTCM/RTCM2Decoder.h          \
          RTCM/RTCM2_2021.h RTCM/rtcm_utils.h                         \
          RTCM3/RTCM3Decoder.h RTCM3/rtcm3torinex.h                   \
          RTCM3/RTCM3coDecoder.h RTCM3/clock_orbit_rtcm.h             \
          RTCM3/ephemeris.h RTCM3/timeutils.h                         \

SOURCES = ntrip2test.cpp bncnetrequest.cpp bncutils.cpp bncrinex.cpp  \
          RTCM/RTCM2.cpp RTCM/RTCM2Decoder.cpp                        \
          RTCM/RTCM2_2021.cpp RTCM/rtcm_utils.cpp                     \
          RTCM3/RTCM3Decoder.cpp RTCM3/rtcm3torinex.c                 \
          RTCM3/RTCM3coDecoder.cpp RTCM3/clock_orbit_rtcm.c           \
          RTCM3/ephemeris.cpp RTCM3/timeutils.cpp                     

QT += network

