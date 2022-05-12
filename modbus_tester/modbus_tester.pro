
TEMPLATE = app
TARGET = modbus_tester
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj

QT -= gui
QT *= widgets xml
QT *= serialbus serialport

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../libs/base \
            $$PWD/../libs/io_bus \
            $$PWD/../libs/base/ui_h

#path for libs files
DEPENDPATH += $$PWD/../libs/base/build \
            $$PWD/../libs/io_bus/build

#include libs
unix:!macx: LIBS += -L$$PWD/../libs/base/build/ -llbase
unix:!macx: LIBS += -L$$PWD/../libs/io_bus/build/ -lliobus


# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/modbusobj.h \
	$$PWD/mbcrckeytable.h \
	$$PWD/mbparams.h \
	$$PWD/mbdeviceemulator.h \
	$$PWD/mbserver.h \
        $$PWD/mbconfigloader.h

SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
        $$PWD/modbusobj.cpp \
	$$PWD/mbdeviceemulator.cpp \
	$$PWD/mbserver.cpp \
        $$PWD/mbconfigloader.cpp



