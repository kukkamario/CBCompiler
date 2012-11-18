#-------------------------------------------------
#
# Project created by QtCreator 2012-05-25T15:50:57
#
#-------------------------------------------------

QT       -= core gui

TARGET = Runtime
TEMPLATE = lib
CONFIG += staticlib
SOURCES += \
    system.cpp \
    cbstring.cpp \
    main.cpp \
    referencecounter.cpp

HEADERS += \
    cbstring.h \
    referencecounter.h \
    cbfunc.h

INCLUDEPATH += "$$(BOOST_INCLUDE)"
LIBS += "$$(BOOST_LIB)"
DEFINES += RUNTIME

QMAKE_CXXFLAGS += -std=c++0x
