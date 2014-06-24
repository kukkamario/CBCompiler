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

DEPENDPATH += "./"


CONFIG(debug, debug|release) {
	DEFINES += _DEBUG DEBUG_OUTPUT
}
win32 {
	INCLUDEPATH += "$$(LLVM_INCLUDE)"
	LIBS +=  -L"$$(LLVM_LIB)"
	LIBS += -lLLVMVectorize -lLLVMipo -lLLVMLinker -lLLVMBitReader -lLLVMBitWriter -lLLVMInstrumentation -lLLVMX86CodeGen -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMArchive -lLLVMAsmParser -lLLVMAsmPrinter -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMMCParser -lLLVMCodeGen -lLLVMX86AsmPrinter -lLLVMX86Info -lLLVMScalarOpts -lLLVMIRReader -lLLVMObjCARCOpts -lLLVMX86Utils -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMMC -lLLVMObject -lLLVMSupport
	LIBS += -lAdvapi32 -lShell32
	win32-msvc { #msvc
		QMAKE_CXXFLAGS += /Zc:wchar_t
		LIBS += -ldbghelp
	} else { #mingw
		LIBS += -lpsapi -limagehlp # = dbghelp (msvc)
		QMAKE_CXXFLAGS += -std=c++11
	}
}

linux {
#    !isEmpty($$(LLVM_LIB)) {
#        LIBS += -L"$$(LLVM_LIB)"
#    }
#    !isEmpty($$(LLVM_INCLUDE)) {
#        INCLUDEPATH += "$$(LLVM_INCLUDE)"
#    }
#	LIBS += -lLLVMVectorize -lLLVMipo -lLLVMLinker -lLLVMBitReader -lLLVMBitWriter -lLLVMInstrumentation -lLLVMX86CodeGen -lLLVMX86AsmParser -lLLVMX86Disassembler -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMArchive -lLLVMAsmParser -lLLVMAsmPrinter -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMMCParser -lLLVMCodeGen -lLLVMX86AsmPrinter -lLLVMX86Info -lLLVMScalarOpts -lLLVMX86Utils -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMMC -lLLVMObject -lLLVMSupport
	LIBS += -L`llvm-config --libdir` `llvm-config --libs all`
	LIBS += -ldl
	QMAKE_CXXFLAGS += -std=c++11
}

macx {
	LIBS += -lLLVMVectorize -lLLVMipo -lLLVMLinker -lLLVMBitReader -lLLVMBitWriter -lLLVMInstrumentation -lLLVMX86CodeGen -lLLVMX86AsmParser -lLLVMX86Disassembler -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMArchive -lLLVMAsmParser -lLLVMAsmPrinter -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMMCParser -lLLVMCodeGen -lLLVMX86AsmPrinter -lLLVMX86Info -lLLVMScalarOpts -lLLVMX86Utils -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMMC -lLLVMObject -lLLVMSupport
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
    constantvalue.cpp \
    constantsymbol.cpp \
    variablesymbol.cpp \
    typesymbol.cpp \
    typepointervaluetype.cpp \
    cbfunction.cpp \
    labelsymbol.cpp \
    builder.cpp \
    settings.cpp \
    typevaluetype.cpp \
    valuetypesymbol.cpp \
    customdatatypedefinitions.cpp \
    customvaluetype.cpp \
    astvisitor.cpp \
    symbolcollector.cpp \
    typeresolver.cpp \
    valuetypecollection.cpp \
    arrayvaluetype.cpp \
    genericarrayvaluetype.cpp \
    functionvaluetype.cpp \
    functionselectorvaluetype.cpp \
    functioncodegenerator.cpp \
    castcostcalculator.cpp

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
    constantvalue.h \
    constantsymbol.h \
    variablesymbol.h \
    typesymbol.h \
    typepointervaluetype.h \
    cbfunction.h \
    labelsymbol.h \
    builder.h \
    global.h \
    warningcodes.h \
    settings.h \
    typevaluetype.h \
    valuetypesymbol.h \
    customdatatypedefinitions.h \
    customvaluetype.h \
    codepoint.h \
    astvisitor.h \
    symbolcollector.h \
    typeresolver.h \
    valuetypecollection.h \
    arrayvaluetype.h \
    genericarrayvaluetype.h \
    functionvaluetype.h \
    liststringjoin.h \
    operationflags.h \
    functionselectorvaluetype.h \
    functioncodegenerator.h \
    castcostcalculator.h
