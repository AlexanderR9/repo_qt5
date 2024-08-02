
TEMPLATE = app
TARGET = poolcalcv3
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= widgets

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../../libs/base \
            $$PWD/../../libs/base/ui_h \
            $$PWD/../../libs/uniswap

#path for libs files
DEPENDPATH += $$PWD/../../libs/base/build \
            $$PWD/../../libs/uniswap/build

#include libs
unix:!macx: LIBS += -L$$PWD/../../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../../libs/uniswap/build/ -lluniswap

# Input
HEADERS += $$PWD/mainform.h \
#    $$PWD/tickobj.h \
    $$PWD/paramswidget.h

SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
#    $$PWD/tickobj.cpp \
    $$PWD/paramswidget.cpp

FORMS += $$PWD/poolparams.ui


