
DEFINES += NO_RTCM3_MAIN 

CONFIG += release console

INCLUDEPATH = . ./RTCM3

HEADERS = bnctime.h   RTCM3/rtcm3torinex.h RTCM3/timeutils.h

SOURCES = test_bnc_qt.cpp \
          bnctime.cpp RTCM3/rtcm3torinex.c RTCM3/timeutils.cpp

QT += network

