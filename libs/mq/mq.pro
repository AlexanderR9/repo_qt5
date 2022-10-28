TARGET = build/lmq
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
MOC_DIR = moc
OBJECTS_DIR = obj

QT -= gui
QT += core widgets xml

INCLUDEPATH += .\
            $$PWD/../base \
            $$PWD

#path for libs files
DEPENDPATH += $$PWD/../base/build \
                $$PWD/lib64

#include libs
unix:!macx: LIBS += -L$$PWD/../base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/lib64/ -lrt


# Input
HEADERS += $$PWD/mq.h \
	$$PWD/mq_global.h \
	$$PWD/mqworker.h


SOURCES += $$PWD/mq.cpp \
	$$PWD/mqworker.cpp



