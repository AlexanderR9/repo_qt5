######################################################################
# Automatically generated by qmake (2.01a) ?? ??? 24 11:38:33 2019
######################################################################

TEMPLATE = app
TARGET = com_tester
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj


QT -= gui
QT *= network widgets xml serialport


NCLUDEPATH += . \
            $$PWD \
            $$PWD/../../libs/base \
            $$PWD/../../libs/io_bus \
            $$PWD/../../libs/base/ui_h

#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build \
            $$PWD/../../libs/io_bus/build

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../../libs/io_bus/build/ -lliobus
            

# Input
HEADERS += mainform.h \
        comobj.h \
    fileworker.h \
    comparams_struct.h

SOURCES += main.cpp \
	mainform.cpp \
        comobj.cpp \
    fileworker.cpp

