#-------------------------------------------------
#
# Project created by QtCreator 2012-11-06T15:59:39
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = CBCompiler
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
win32 {
	INCLUDEPATH += "$$(LLVM_INCLUDE)"
	CONFIG(release, debug|release) {
		LIBS +=  -L"$$(LLVM_LIB)"/Release
	} else {
		LIBS +=  -L"$$(LLVM_LIB)"/Debug
	}
	DEPENDPATH += "./"
	LIBS += -lLLVMVectorize -lLLVMipo -lLLVMLinker -lLLVMBitReader -lLLVMBitWriter -lLLVMInstrumentation -lLLVMX86CodeGen -lLLVMX86AsmParser -lLLVMX86Disassembler -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMArchive -lLLVMAsmParser -lLLVMAsmPrinter -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMMCParser -lLLVMCodeGen -lLLVMX86AsmPrinter -lLLVMX86Info -lLLVMScalarOpts -lLLVMX86Utils -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMMC -lLLVMObject -lLLVMSupport

}

SOURCES += main.cpp \
    lexer.cpp \
    token.cpp \
    errorhandler.cpp \
    parser.cpp \
    scope.cpp \
    symbol.cpp \
    functionsymbol.cpp \
    valuetype.cpp \
    value.cpp \
    abstractsyntaxtree.cpp \
    function.cpp

HEADERS += \
    lexer.h \
    token.h \
    errorhandler.h \
    parser.h \
    scope.h \
    symbol.h \
    functionsymbol.h \
    valuetype.h \
    value.h \
    abstractsyntaxtree.h \
    function.h \
    errorcodes.h
