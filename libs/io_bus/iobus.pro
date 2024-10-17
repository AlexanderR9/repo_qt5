TARGET = build/liobus
TEMPLATE = lib
DEFINES += LIB_LIBRARY
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT += serialbus serialport

INCLUDEPATH += .\
	    $$PWD


# Input
HEADERS += $$PWD/mbadu.h \
	$$PWD/mbcrckeytable.h \
	$$PWD/mbtcpslavebase.h \
	$$PWD/comparams.h \
	$$PWD/comportobject.h \
	$$PWD/iobus_global.h \
	$$PWD/mbslaveserverbase.h \
	$$PWD/mbtcpmasterbase.h \
        $$PWD/mbtcpstatistic.h


SOURCES += $$PWD/mbadu.cpp \
	$$PWD/mbslaveserverbase.cpp \
	$$PWD/comportobject.cpp \
	$$PWD/mbtcpslavebase.cpp \
	$$PWD/mbtcpmasterbase.cpp \
        $$PWD/mbtcpstatistic.cpp

