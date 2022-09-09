TARGET = build/lfx
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT *= widgets xml

INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD/../base/ui_h \
            $$PWD


#path for libs files
DEPENDPATH += $$PWD/../base/build

#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase


# Input
HEADERS += $$PWD/fxbar.h \
        $$PWD/fxbarcontainer.h \
        $$PWD/fxenums.h \
        $$PWD/fxdataloader.h \
        $$PWD/fxabstracttest.h \
        $$PWD/fxchartwidget.h \
        $$PWD/fxdataloaderwidget.h


SOURCES += $$PWD/fxbar.cpp \
        $$PWD/fxbarcontainer.cpp \
        $$PWD/fxenums.cpp \
        $$PWD/fxabstracttest.cpp \
        $$PWD/fxdataloader.cpp \
        $$PWD/fxchartwidget.cpp \
        $$PWD/fxdataloaderwidget.cpp

