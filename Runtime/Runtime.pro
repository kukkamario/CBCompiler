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
    referencecounter.h

#LLVM_SOURCE += ../atomic_operations.ll
#build_llvm_source.target = llvm-as
#build_llvm_source.commands = $$build_llvm_source.target -o llvm.bc $$LLVM_SOURCE
#QMAKE_PRE_LINK += $$build_llvm_source

OBJECTS += llvm.bc

INCLUDEPATH += "$$(BOOST_INCLUDE)"


QMAKE_CXXFLAGS += -std=c++0x

OTHER_FILES += \
    atomic_operations.ll
