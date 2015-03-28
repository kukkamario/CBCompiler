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
CONFIG += c++11

win32 {
	INCLUDEPATH += "$$(LLVM_INCLUDE)"
	LIBS +=  -L"$$(LLVM_LIB)"
	#LIBS += -lLLVMInstrumentation -lLLVMIRReader -lLLVMAsmParser -lLLVMDebugInfo -lLLVMOption -lLLVMLTO -lLLVMLinker -lLLVMipo -lLLVMVectorize -lLLVMBitWriter -lLLVMBitReader -lLLVMTableGen -lLLVMXCoreDisassembler -lLLVMXCoreCodeGen -lLLVMXCoreDesc -lLLVMXCoreInfo -lLLVMXCoreAsmPrinter -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMSystemZDisassembler -lLLVMSystemZCodeGen -lLLVMSystemZAsmParser -lLLVMSystemZDesc -lLLVMSystemZInfo -lLLVMSystemZAsmPrinter -lLLVMSparcCodeGen -lLLVMSparcDesc -lLLVMSparcInfo -lLLVMR600CodeGen -lLLVMR600Desc -lLLVMR600Info -lLLVMR600AsmPrinter -lLLVMPowerPCCodeGen -lLLVMPowerPCAsmParser -lLLVMPowerPCDesc -lLLVMPowerPCInfo -lLLVMPowerPCAsmPrinter -lLLVMNVPTXCodeGen -lLLVMNVPTXDesc -lLLVMNVPTXInfo -lLLVMNVPTXAsmPrinter -lLLVMMSP430CodeGen -lLLVMMSP430Desc -lLLVMMSP430Info -lLLVMMSP430AsmPrinter -lLLVMMipsDisassembler -lLLVMMipsCodeGen -lLLVMMipsAsmParser -lLLVMMipsDesc -lLLVMMipsInfo -lLLVMMipsAsmPrinter -lLLVMHexagonCodeGen -lLLVMHexagonAsmPrinter -lLLVMHexagonDesc -lLLVMHexagonInfo -lLLVMCppBackendCodeGen -lLLVMCppBackendInfo -lLLVMARMDisassembler -lLLVMARMCodeGen -lLLVMARMAsmParser -lLLVMARMDesc -lLLVMARMInfo -lLLVMARMAsmPrinter -lLLVMAArch64Disassembler -lLLVMAArch64CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMAArch64AsmParser -lLLVMAArch64Desc -lLLVMAArch64Info -lLLVMAArch64AsmPrinter -lLLVMAArch64Utils -lgtest_main -lgtest -lLLVMMCDisassembler -lLLVMMCParser -lLLVMInterpreter -lLLVMMCJIT -lLLVMJIT -lLLVMCodeGen -lLLVMObjCARCOpts -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport
	LIBS += -lLLVMLTO -lLLVMObjCARCOpts -lLLVMLinker -lLLVMipo -lLLVMVectorize -lLLVMBitWriter -lLLVMIRReader -lLLVMAsmParser -lLLVMXCoreDisassembler -lLLVMXCoreCodeGen -lLLVMXCoreDesc -lLLVMXCoreInfo -lLLVMXCoreAsmPrinter -lLLVMSystemZDisassembler -lLLVMSystemZCodeGen -lLLVMSystemZAsmParser -lLLVMSystemZDesc -lLLVMSystemZInfo -lLLVMSystemZAsmPrinter -lLLVMSparcDisassembler -lLLVMSparcCodeGen -lLLVMSparcAsmParser -lLLVMSparcDesc -lLLVMSparcInfo -lLLVMSparcAsmPrinter -lLLVMPowerPCDisassembler -lLLVMPowerPCCodeGen -lLLVMPowerPCAsmParser -lLLVMPowerPCDesc -lLLVMPowerPCInfo -lLLVMPowerPCAsmPrinter -lLLVMNVPTXCodeGen -lLLVMNVPTXDesc -lLLVMNVPTXInfo -lLLVMNVPTXAsmPrinter -lLLVMMSP430CodeGen -lLLVMMSP430Desc -lLLVMMSP430Info -lLLVMMSP430AsmPrinter -lLLVMMipsDisassembler -lLLVMMipsCodeGen -lLLVMMipsAsmParser -lLLVMMipsDesc -lLLVMMipsInfo -lLLVMMipsAsmPrinter -lLLVMHexagonCodeGen -lLLVMHexagonAsmPrinter -lLLVMHexagonDesc -lLLVMHexagonInfo -lLLVMCppBackendCodeGen -lLLVMCppBackendInfo -lLLVMARMDisassembler -lLLVMARMCodeGen -lLLVMARMAsmParser -lLLVMARMDesc -lLLVMARMInfo -lLLVMARMAsmPrinter -lLLVMAArch64Disassembler -lLLVMAArch64CodeGen -lLLVMAArch64AsmParser -lLLVMAArch64Desc -lLLVMAArch64Info -lLLVMAArch64AsmPrinter -lLLVMAArch64Utils -lgtest_main -lgtest -lLLVMTableGen -lLLVMDebugInfo -lLLVMOption -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMJIT -lLLVMLineEditor -lLLVMMCAnalysis -lLLVMMCDisassembler -lLLVMInstrumentation -lLLVMInterpreter -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMProfileData -lLLVMMCJIT -lLLVMTarget -lLLVMRuntimeDyld -lLLVMObject -lLLVMMCParser -lLLVMBitReader -lLLVMExecutionEngine -lLLVMMC -lLLVMCore -lLLVMSupport
#LIBS += -lLLVMInstrumentation -lLLVMIRReader -lLLVMAsmParser -lLLVMDebugInfo -lLLVMOption -lLLVMLTO -lLLVMLinker -lLLVMipo -lLLVMVectorize -lLLVMBitWriter -lLLVMBitReader -lLLVMTableGen -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lgtest_main -lgtest -lLLVMMCDisassembler -lLLVMMCParser -lLLVMInterpreter -lLLVMMCJIT -lLLVMJIT -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport
	LIBS += -lAdvapi32 -lShell32
	win32-msvc { #msvc
		QMAKE_CXXFLAGS += /Zc:wchar_t
		LIBS += -ldbghelp
	} else { #mingw
		LIBS += -lpsapi -limagehlp # = dbghelp (msvc)
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
    castcostcalculator.cpp \
    constantexpressionevaluator.cpp \
    genericstructvaluetype.cpp \
    structvaluetype.cpp \
    nullvaluetype.cpp

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
    castcostcalculator.h \
    constantexpressionevaluator.h \
    structvaluetype.h \
    genericstructvaluetype.h \
    nullvaluetype.h
