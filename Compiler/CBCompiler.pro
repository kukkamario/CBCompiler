#-------------------------------------------------
#
# Project created by QtCreator 2012-11-06T15:59:39
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = CBCompiler
CONFIG   += console precompiled_header
CONFIG   -= app_bundle

PRECOMPILED_HEADER = llvm.h
TEMPLATE = app

DEFINES += __STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS



CONFIG(debug, debug|release) {
	DEFINES += _DEBUG
}
win32 {
	INCLUDEPATH += "$$(LLVM_INCLUDE)"
	LIBS +=  -L"$$(LLVM_LIB)"
	DEPENDPATH += "./"
	LIBS += -lLLVMVectorize -lLLVMipo -lLLVMLinker -lLLVMBitReader -lLLVMBitWriter -lLLVMInstrumentation -lLLVMX86CodeGen -lLLVMX86AsmParser -lLLVMX86Disassembler -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMArchive -lLLVMAsmParser -lLLVMAsmPrinter -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMMCParser -lLLVMCodeGen -lLLVMX86AsmPrinter -lLLVMX86Info -lLLVMScalarOpts -lLLVMX86Utils -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMMC -lLLVMObject -lLLVMSupport
	LIBS += -lAdvapi32 -lShell32 -ldbghelp
	QMAKE_CXXFLAGS += /Zc:wchar_t
}

unix {
    #!isEmpty($$(LLVM_LIB)) {
        LIBS += -L"$$(LLVM_LIB)"
    #}
    !isEmpty($$(LLVM_INCLUDE)) {
        INCLUDEPATH += "$$(LLVM_INCLUDE)"
    }
	#LIBS += -lLLVMVectorize -lLLVMipo -lLLVMLinker -lLLVMBitReader -lLLVMBitWriter -lLLVMInstrumentation -lLLVMX86CodeGen -lLLVMX86AsmParser -lLLVMX86Disassembler -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMArchive -lLLVMAsmParser -lLLVMAsmPrinter -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMMCParser -lLLVMCodeGen -lLLVMX86AsmPrinter -lLLVMX86Info -lLLVMScalarOpts -lLLVMX86Utils -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMMC -lLLVMObject -lLLVMSupport
	LIBS += -lLLVM-3.2
	LIBS += -ldl
    DEPENDPATH += "./"
}

DESTDIR = $$PWD/../bin

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
    function.cpp \
    runtime.cpp \
    runtimefunction.cpp \
    intvaluetype.cpp \
    codegenerator.cpp \
    stringpool.cpp \
    stringvaluetype.cpp \
    floatvaluetype.cpp \
    shortvaluetype.cpp \
    bytevaluetype.cpp \
    booleanvaluetype.cpp \
    constantexpressionevaluator.cpp \
    constantvalue.cpp \
    constantsymbol.cpp \
    variablesymbol.cpp \
    typesymbol.cpp \
    typepointervaluetype.cpp \
    symbolcollectortypechecker.cpp \
    arraysymbol.cpp \
    cbfunction.cpp \
    functioncodegenerator.cpp \
    expressioncodegenerator.cpp \
    labelsymbol.cpp \
    builder.cpp

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
    errorcodes.h \
    runtime.h \
    runtimefunction.h \
    llvm.h \
    intvaluetype.h \
    codegenerator.h \
    stringpool.h \
    stringvaluetype.h \
    floatvaluetype.h \
    shortvaluetype.h \
    bytevaluetype.h \
    booleanvaluetype.h \
    constantexpressionevaluator.h \
    constantvalue.h \
    unionwrapper.h \
    constantsymbol.h \
    variablesymbol.h \
    typesymbol.h \
    typepointervaluetype.h \
    symbolcollectortypechecker.h \
    conversionhelper.h \
    arraysymbol.h \
    cbfunction.h \
    functioncodegenerator.h \
    expressioncodegenerator.h \
    labelsymbol.h \
    builder.h \
    global.h \
    warningcodes.h
