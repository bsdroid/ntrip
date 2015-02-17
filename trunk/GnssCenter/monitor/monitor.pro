
TEMPLATE             = lib
CONFIG              += plugin debug
TARGET               = $$qtLibraryTarget(gnsscenter_monitor)
QT                  += svg
INCLUDEPATH         += ../qwt ../main
DESTDIR              = ../plugins
QMAKE_LIBDIR        += ../qwt $(HOME)/Software/thrift/lib
LIBS                 = -lqwt -lthrift
INCLUDEPATH         += $(HOME)/Software/thrift/include/thrift
DEFINES             += HAVE_INTTYPES_H HAVE_NETINET_IN_H

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

thrift.target   = gen-cpp
thrift.commands = "thrift -r -gen cpp $(THRIFT_DIR)/rtnet.thrift"
thrift.depends  = $(THRIFT_DIR)/rtnet.thrift $(THRIFT_DIR)/rtnet_data.thrift

PRE_TARGETDEPS      += gen-cpp
QMAKE_EXTRA_TARGETS += thrift

HEADERS   = monitor.h      \
            dlgconf.h      \
            utils.h        \
            const.h        \
            worldplot.h    \
            thriftclient.h 

SOURCES   = monitor.cpp      \
            dlgconf.cpp      \
            utils.cpp        \
            const.cpp        \
            worldplot.cpp    \
            thriftclient.cpp \
            ../main/settings.cpp \
            gen-cpp/RtnetData.cpp \
            gen-cpp/rtnet_constants.cpp gen-cpp/rtnet_types.cpp \
            gen-cpp/rtnet_data_constants.cpp gen-cpp/rtnet_data_types.cpp


RESOURCES = monitor.qrc
