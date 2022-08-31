
TEMPLATE = app
TARGET = tcpemulator
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
#UI_DIR=ui

QT -= gui
QT *= network widgets xml testlib

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../libs/base \
            $$PWD/../libs/base/ui_h \
            $$PWD/../libs/tcp \
            $$PWD/../libs/tcp/ui \


#path for libs files
DEPENDPATH += $$PWD/../libs/base/build
DEPENDPATH += $$PWD/../libs/tcp/build


#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/tcp/build/ -lltcp



# Input
HEADERS += $$PWD/mainform.h \
#	    $$PWD/tcpstatuswidget.h \
	    $$PWD/dtsstructs.h
	    

SOURCES += $$PWD/main.cpp \
#	$$PWD/tcpstatuswidget.cpp \
	$$PWD/mainform.cpp

#FORMS += $$PWD/tcpstatuswidget.ui



