TARGET = build/ltcp
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT *= network widgets xml testlib

INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD


#path for libs files
DEPENDPATH += $$PWD/../base/build
#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase


# Input
HEADERS += $$PWD/tcpserverobj.h \
	$$PWD/tcpclientobj.h \
	$$PWD/tcp_global.h

SOURCES += $$PWD/tcpserverobj.cpp \
	$$PWD/tcpclientobj.cpp


