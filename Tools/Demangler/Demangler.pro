TEMPLATE = app
QT = core
CONFIG += console
CONFIG -= app_bundle

SOURCES += main.cpp

DEFINES += __STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS
TARGET = demangler
DESTDIR = $$PWD/../../bin

win32 {
	INCLUDEPATH += "$$(LLVM_INCLUDE)"
	LIBS +=  -L"$$(LLVM_LIB)"
	LIBS += -lLLVMInstrumentation -lLLVMIRReader -lLLVMAsmParser -lLLVMDebugInfo -lLLVMOption -lLLVMLTO -lLLVMLinker -lLLVMipo -lLLVMVectorize -lLLVMBitWriter -lLLVMBitReader -lLLVMTableGen -lLLVMXCoreDisassembler -lLLVMXCoreCodeGen -lLLVMXCoreDesc -lLLVMXCoreInfo -lLLVMXCoreAsmPrinter -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMSystemZDisassembler -lLLVMSystemZCodeGen -lLLVMSystemZAsmParser -lLLVMSystemZDesc -lLLVMSystemZInfo -lLLVMSystemZAsmPrinter -lLLVMSparcCodeGen -lLLVMSparcDesc -lLLVMSparcInfo -lLLVMR600CodeGen -lLLVMR600Desc -lLLVMR600Info -lLLVMR600AsmPrinter -lLLVMPowerPCCodeGen -lLLVMPowerPCAsmParser -lLLVMPowerPCDesc -lLLVMPowerPCInfo -lLLVMPowerPCAsmPrinter -lLLVMNVPTXCodeGen -lLLVMNVPTXDesc -lLLVMNVPTXInfo -lLLVMNVPTXAsmPrinter -lLLVMMSP430CodeGen -lLLVMMSP430Desc -lLLVMMSP430Info -lLLVMMSP430AsmPrinter -lLLVMMipsDisassembler -lLLVMMipsCodeGen -lLLVMMipsAsmParser -lLLVMMipsDesc -lLLVMMipsInfo -lLLVMMipsAsmPrinter -lLLVMHexagonCodeGen -lLLVMHexagonAsmPrinter -lLLVMHexagonDesc -lLLVMHexagonInfo -lLLVMCppBackendCodeGen -lLLVMCppBackendInfo -lLLVMARMDisassembler -lLLVMARMCodeGen -lLLVMARMAsmParser -lLLVMARMDesc -lLLVMARMInfo -lLLVMARMAsmPrinter -lLLVMAArch64Disassembler -lLLVMAArch64CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMAArch64AsmParser -lLLVMAArch64Desc -lLLVMAArch64Info -lLLVMAArch64AsmPrinter -lLLVMAArch64Utils -lgtest_main -lgtest -lLLVMMCDisassembler -lLLVMMCParser -lLLVMInterpreter -lLLVMMCJIT -lLLVMJIT -lLLVMCodeGen -lLLVMObjCARCOpts -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport
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
	LIBS += -L`llvm-config --libdir` `llvm-config --libs bitreader asmparser`
	LIBS += -ldl -lpthread
	QMAKE_CXXFLAGS += -std=c++11
}

macx {
	LIBS += -lLLVMVectorize -lLLVMipo -lLLVMLinker -lLLVMBitReader -lLLVMBitWriter -lLLVMInstrumentation -lLLVMX86CodeGen -lLLVMX86AsmParser -lLLVMX86Disassembler -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMArchive -lLLVMAsmParser -lLLVMAsmPrinter -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMMCParser -lLLVMCodeGen -lLLVMX86AsmPrinter -lLLVMX86Info -lLLVMScalarOpts -lLLVMX86Utils -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMMC -lLLVMObject -lLLVMSupport
}
