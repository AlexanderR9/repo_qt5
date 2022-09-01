
QT += widgets xml testlib
QT -= gui

TARGET = build/lbase
TEMPLATE = lib
DEFINES += LIB_LIBRARY
UI_DIR += ui_h
MOC_DIR = moc
OBJECTS_DIR = obj

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += .\
	    $$PWD \
	    $$PWD/../src


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
    lsimpleobj.cpp

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
    lsimpleobj.h

#unix {
#   target.path = /usr/lib
#   INSTALLS += target
#

FORMS += lsimpledialog.ui

RESOURCES += $$PWD/../src/icons.qrc





