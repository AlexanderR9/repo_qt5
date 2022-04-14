######################################################################
# Automatically generated by qmake (2.01a) ?? ??? 24 11:38:33 2019
######################################################################

TEMPLATE = app
TARGET = modbus_tester
DEFINES += QT_DEPRECATED_WARNINGS
MOC_DIR=moc
OBJECTS_DIR=obj

QT -= gui
QT *= network widgets xml
QT += serialbus serialport

INCLUDEPATH += . \
            $$PWD \
            $$PWD/../lib \
            $$PWD/../lib/ui_h

DEPENDPATH += $$PWD/../lib/build
unix:!macx: LIBS += -L$$PWD/../lib/build -llib

# Input
HEADERS += $$PWD/mainform.h \
        $$PWD/modbusobj.h \
	$$PWD/mbcrckeytable.h \
	$$PWD/comparams_struct.h \
	$$PWD/mbslaveserverbase.h \
	$$PWD/mbdeviceemulator.h \
	$$PWD/mbserver.h \
	$$PWD/mbadu.h

SOURCES += $$PWD/main.cpp \
	$$PWD/mainform.cpp \
        $$PWD/modbusobj.cpp \
	$$PWD/mbslaveserverbase.cpp \
	$$PWD/mbdeviceemulator.cpp \
	$$PWD/mbserver.cpp \
	$$PWD/mbadu.cpp

