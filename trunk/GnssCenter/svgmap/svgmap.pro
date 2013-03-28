
TEMPLATE             = lib
CONFIG              += plugin static debug
TARGET               = $$qtLibraryTarget(GnssCenter_svgMap)
QT                  += svg
INCLUDEPATH         += ../qwt ../main

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

HEADERS   = svgmap.h

SOURCES   = svgmap.cpp

RESOURCES = svgmap.qrc
