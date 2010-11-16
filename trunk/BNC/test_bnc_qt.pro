
DEFINES += NO_RTCM3_MAIN 

CONFIG += release console

INCLUDEPATH = . ./RTCM3

HEADERS = bnctime.h

SOURCES = test_bnc_qt.cpp

QT += network

