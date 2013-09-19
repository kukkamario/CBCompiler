TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

DEFINES += __STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS
TARGET = demangler
DESTDIR = $$PWD/../../bin

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
	}
}

linux {
	LIBS += -L`llvm-config --libdir` `llvm-config --libs bitreader asmparser`
	LIBS += -ldl -lpthread
}

macx {
	LIBS += -lLLVMVectorize -lLLVMipo -lLLVMLinker -lLLVMBitReader -lLLVMBitWriter -lLLVMInstrumentation -lLLVMX86CodeGen -lLLVMX86AsmParser -lLLVMX86Disassembler -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMArchive -lLLVMAsmParser -lLLVMAsmPrinter -lLLVMSelectionDAG -lLLVMX86Desc -lLLVMMCParser -lLLVMCodeGen -lLLVMX86AsmPrinter -lLLVMX86Info -lLLVMScalarOpts -lLLVMX86Utils -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMMC -lLLVMObject -lLLVMSupport
}
