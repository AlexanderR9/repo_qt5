
TEMPLATE = app
TARGET = fxtester
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj
UI_DIR=ui

QT -= gui
QT *= network widgets xml testlib

INCLUDEPATH += . \
            $$PWD \
            $$PWD/ui \
            $$PWD/../libs/base \
            $$PWD/../libs/base/ui_h \
            $$PWD/../libs/fx


#path for libs files
DEPENDPATH += $$PWD/../libs/base/build \
            $$PWD/../libs/fx/build

#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/fx/build/ -llfx


# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/fxcentralwidget.h \
        $$PWD/fxtestpage.h \
        $$PWD/fxqualdatapage.h \
        $$PWD/fxtest.h \
        $$PWD/fxtesterobj.h


SOURCES += $$PWD/main.cpp \
        $$PWD/mainform.cpp \
        $$PWD/fxcentralwidget.cpp \
        $$PWD/fxtestpage.cpp \
        $$PWD/fxqualdatapage.cpp \
        $$PWD/fxtest.cpp \
        $$PWD/fxtesterobj.cpp


FORMS += $$PWD/ui/fxcentralwidget.ui

