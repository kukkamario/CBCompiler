#ifndef LLVM_H
#define LLVM_H

#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/PassManagers.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/Path.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/Mangler.h>
#include <llvm/ADT/Triple.h>
#if LLVM_VERSION_MAJOR != 3
	#error "CBCompiler requires llvm version 3.*"
#endif
#if LLVM_VERSION_MINOR < 3
	#include <llvm/LLVMContext.h>
	#include <llvm/Module.h>
	#include <llvm/DerivedTypes.h>
	#include <llvm/Constants.h>
	#include <llvm/GlobalVariable.h>
	#include <llvm/Function.h>
	#include <llvm/CallingConv.h>
	#include <llvm/BasicBlock.h>
	#include <llvm/Instructions.h>
	#include <llvm/InlineAsm.h>
	#include <llvm/Support/IRReader.h>
	#if LLVM_VERSION_MINOR == 2
		#include <llvm/IRBuilder.h>
	#else
		#include <llvm/Support/IRBuilder.h>
	#endif
#else

	#include <llvm/IR/IRBuilder.h>
	#include <llvm/IR/LLVMContext.h>
	#include <llvm/IR/Module.h>
	#include <llvm/IR/DerivedTypes.h>
	#include <llvm/IR/Constants.h>
	#include <llvm/IR/GlobalVariable.h>
	#include <llvm/IR/Function.h>
	#include <llvm/IR/CallingConv.h>
	#include <llvm/IR/BasicBlock.h>
	#include <llvm/IR/Instructions.h>
	#include <llvm/IR/InlineAsm.h>
	#include <llvm/IRReader/IRReader.h>
	#include <llvm/Support/SourceMgr.h>
#endif

#include <llvm/Support/DynamicLibrary.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Linker.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/system_error.h>
#include <llvm/Assembly/Parser.h>

#include <llvm/Object/COFF.h>
#include <llvm/Object/ObjectFile.h>
#endif // LLVM_H
