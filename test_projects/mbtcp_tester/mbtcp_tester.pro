######################################################################
# Automatically generated by qmake (2.01a) ?? ??? 24 11:38:33 2019
######################################################################


TEMPLATE = app
TARGET = mbtcp_tester
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui


QT -= gui
QT *= network widgets


INCLUDEPATH += . \
            $$PWD \
            $$PWD/../../libs \
            $$PWD/../../libs/base \
            $$PWD/../../libs/io_bus \
            $$PWD/../../libs/tcp \
            $$PWD/../../libs/base/ui_h

#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build \
            $$PWD/../../libs/tcp/build \
            $$PWD/../../libs/io_bus/build

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../../libs/io_bus/build/ -lliobus
unix:!macx: LIBS += -L$$PWD/../../libs/tcp/build/ -lltcp
            

# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/mbtcpobj.h \
	$$PWD/exchangestatewidget.h \
	$$PWD/reqrespwidget.h \
        $$PWD/mbtcpcentralwidget.h

SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
        $$PWD/mbtcpobj.cpp \
	$$PWD/exchangestatewidget.cpp \
	$$PWD/reqrespwidget.cpp \
        $$PWD/mbtcpcentralwidget.cpp
