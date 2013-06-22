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
    referencecounter.cpp \
    math.cpp \
    error.cpp \
    window.cpp \
    rendertarget.cpp \
    gfx.cpp

HEADERS += \
    cbstring.h \
    referencecounter.h \
    common.h \
    error.h \
    window.h \
    rendertarget.h \
    system.h \
    gfx.h

win32 {
    LLVM_FILES += atomic_operations_mingw.ll
}
linux-g++-32 {
	LLVM_FILES += atomic_operations_gcc-32.ll
}
linux-g++-64 {
	LLVM_FILES += atomic_operations_gcc-64.ll
}

QMAKE_CC = clang
QMAKE_CXX = clang++
QMAKE_CFLAGS = -emit-llvm
QMAKE_CXXFLAGS = -emit-llvm

linux-g++-64 {
	QMAKE_LIB = llvm-link -o
	QMAKE_AR = llvm-link -o  #llvm-ar cqs
}
linux-g++-32 {
	QMAKE_LIB = llvm-link -o
	QMAKE_AR = llvm-link -o  #llvm-ar cqs
}


QMAKE_RUN_CC		= $(CC) $(CCFLAGS) $(INCPATH) -c $src -o $obj
QMAKE_RUN_CC_IMP	= $(CC) $(CCFLAGS) $(INCPATH) -c $< -o $@
QMAKE_RUN_CXX		= $(CXX) $(CXXFLAGS) $(INCPATH) -c $src -o $obj
QMAKE_RUN_CXX_IMP	= $(CXX) $(CXXFLAGS) $(INCPATH) -c $< -o $@

QMAKE_EXT_OBJ           = .bc
QMAKE_EXT_RES           = _res.bc

#QMAKE_PREFIX_STATICLIB  = lib
QMAKE_EXTENSION_STATICLIB = bc

llvm_compiler.output  = ${QMAKE_FILE_BASE}.bc
llvm_compiler.commands = llvm-as ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
llvm_compiler.input = LLVM_FILES
QMAKE_EXTRA_COMPILERS += llvm_compiler

win32 {
	INCLUDEPATH += $$(BOOST_INCLUDE)
	INCLUDEPATH += $$(ALLEGRO_INCLUDE)

	#overrides default link
	#and creates warning... :/
	#TODO: better solution?
	win_link.target = $(DESTDIR_TARGET)
	win_link.commands = llvm-link -o $(DESTDIR_TARGET) $(OBJECTS)
	QMAKE_EXTRA_TARGETS += win_link
}

DESTDIR = $$PWD/../bin/runtime
QMAKE_CXXFLAGS += -std=c++0x

DEFINES += ALLEGRO_STATIC

OTHER_FILES += $$LLVM_FILES
