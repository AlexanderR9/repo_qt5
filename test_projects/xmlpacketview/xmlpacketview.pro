
TEMPLATE = app
TARGET = packetview
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= widgets xml

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../../libs/base \
            $$PWD/../../libs/base/ui_h \
            $$PWD/../../libs/xmlpack


#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build \
            $$PWD/../../libs/xmlpack/build

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../../libs/xmlpack/build/ -llxmlpack


# Input
HEADERS += $$PWD/mainform.h \
	$$PWD/viewwidget.h


SOURCES += $$PWD/main.cpp \
        $$PWD/mainform.cpp \
	$$PWD/viewwidget.cpp


