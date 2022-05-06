
QT -= gui
QT += serialbus serialport


TARGET = build/liobus
TEMPLATE = lib
DEFINES += LIB_LIBRARY
MOC_DIR = moc
OBJECTS_DIR = obj

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += .\
	    $$PWD


# Input
HEADERS += $$PWD/mbadu.h \
	$$PWD/mbcrckeytable.h \
	$$PWD/mbslaveserverbase.h \
	$$PWD/comparams.h \
	$$PWD/comportobject.h \
	$$PWD/iobus_global.h


SOURCES += $$PWD/mbadu.cpp \
	$$PWD/mbslaveserverbase.cpp \
	$$PWD/comportobject.cpp


