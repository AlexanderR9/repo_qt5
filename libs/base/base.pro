
TARGET = build/lbase
TEMPLATE = lib
DEFINES += LIB_LIBRARY QT_DEPRECATED_WARNINGS
UI_DIR += ui_h
MOC_DIR = moc
OBJECTS_DIR = obj


QT -= gui
QT += widgets xml testlib


INCLUDEPATH += .\
	    $$PWD \
	    $$PWD/../src

# inputs
HEADERS += \
    base_global.h \ 
    lmath.h \
    lfile.h \
    ltime.h \
    lprotocol.h \
    lsearch.h \
    lsplash.h \
    ltable.h \
    lstatic.h \
    lsimpledialog.h \
    lchart.h \
    lcommonsettings.h \
    lmainwidget.h \
    lsimpleobj.h \
    lsimplewidget.h

SOURCES += \
    lmath.cpp \
    lfile.cpp \
    ltime.cpp \
    lprotocol.cpp \
    lsearch.cpp \
    lsplash.cpp \
    ltable.cpp \
    lstatic.cpp \
    lsimpledialog.cpp \
    lchart.cpp \
    lcommonsettings.cpp \
    lmainwidget.cpp \
    lsimpleobj.cpp \
    lsimplewidget.cpp

FORMS += lsimpledialog.ui

RESOURCES += $$PWD/../src/icons.qrc





