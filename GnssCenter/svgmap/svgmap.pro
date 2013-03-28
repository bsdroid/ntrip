
TEMPLATE             = lib
CONFIG              += plugin debug
TARGET               = $$qtLibraryTarget(gnsscenter_svgmap)
QT                  += svg
INCLUDEPATH         += ../qwt ../main
DESTDIR              = ../main/plugins

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

HEADERS   = svgmap.h

SOURCES   = svgmap.cpp

RESOURCES = svgmap.qrc
