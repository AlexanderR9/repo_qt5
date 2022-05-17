
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
            $$PWD/../libs/web

#path for libs files
DEPENDPATH += $$PWD/../libs/base/build \
            $$PWD/../libs/web/build


#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/web/build/ -llweb

# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/lbot.h \
        $$PWD/tgsender.h

SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
        $$PWD/lbot.cpp \
        $$PWD/tgsender.cpp





