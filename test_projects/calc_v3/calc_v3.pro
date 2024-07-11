
TEMPLATE = app
TARGET = calcv3
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= network widgets xml
#QT += webenginewidgets testlib

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../../libs/base \
            $$PWD/../../libs/base/ui_h
#            $$PWD/../../libs/web \

#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build
#            $$PWD/../../libs/web/build

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase
#unix:!macx: LIBS += -L$$PWD/../../libs/web/build/ -llweb

# Input
HEADERS += $$PWD/mainform.h \
    $$PWD/tickobj.h \
    $$PWD/paramswidget.h

SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
    $$PWD/tickobj.cpp \
    $$PWD/paramswidget.cpp

FORMS += $$PWD/poolparams.ui


