
TARGET = ../bnc

CONFIG -= release
CONFIG += debug

include(src.pri)

HEADERS +=             app.h

SOURCES += bncmain.cpp app.cpp
