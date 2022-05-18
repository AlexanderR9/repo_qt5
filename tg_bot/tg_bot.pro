
TEMPLATE = app
TARGET = tg_bot
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj

QT -= gui
QT *= network widgets xml

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../libs/base \
            $$PWD/../libs/base/ui_h \
            $$PWD/../libs/tg



#path for libs files
DEPENDPATH += $$PWD/../libs/base/build \
            $$PWD/../libs/tg/build

#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/tg/build/ -lltg

# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/lbot.h

SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
        $$PWD/lbot.cpp



