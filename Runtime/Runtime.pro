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
    main.cpp

HEADERS += \
    cbstring.h

INCLUDEPATH += "$$(BOOST_INCLUDE)"
LIBS += "$$(BOOST_LIB)"
DEFINES += RUNTIME

TARGET_EXT = .bc
QMAKE_EXT_OBJ = .bc
QMAKE_CXXFLAGS -= -mthreads
QMAKE_CXXFLAGS += -emit-llvm
QMAKE_CXX = clang++
QMAKE_CC = clang
QMAKE_LIB = llvm-link -o
QMAKE_RUN_CXX = $(CXX) $(CXXFLAGS) $(INCPATH) -c $src -o $obj
QMAKE_RUN_CC = $(CC) $(CCFLAGS) $(INCPATH) -c $src -o $obj
