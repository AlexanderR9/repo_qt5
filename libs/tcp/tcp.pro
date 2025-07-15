TARGET = build/ltcp
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj
UI_DIR=ui

QT -= gui
QT *= network widgets xml testlib

INCLUDEPATH += .\
            $$PWD/ui \
            $$PWD/../base \
            $$PWD


#path for libs files
DEPENDPATH += $$PWD/../base/build
#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase


# Input
HEADERS += $$PWD/tcpserverobj.h \
	$$PWD/tcpclientobj.h \
	$$PWD/tcpstatuswidget.h \
	$$PWD/tcp_global.h \
        $$PWD/httpserverobj.h

SOURCES += $$PWD/tcpserverobj.cpp \
	$$PWD/tcpstatuswidget.cpp \
	$$PWD/tcpclientobj.cpp \
        $$PWD/httpserverobj.cpp

FORMS += $$PWD/ui/tcpstatuswidget.ui



