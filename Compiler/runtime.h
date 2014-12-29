#ifndef RUNTIME_H
#define RUNTIME_H
#include <QFile>
#include "runtimefunction.h"
#include "llvm.h"
#include "valuetype.h"
#include <map>
#include <QMultiMap>
#include <QHash>
#include "valuetypecollection.h"
#include "codepoint.h"
class ErrorHandler;

class IntValueType;
class StringValueType;
class FloatValueType;
class ShortValueType;
class ByteValueType;
class BooleanValueType;
class StringPool;
class TypePointerCommonValueType;
class GenericArrayValueType;
class TypeValueType;
class Scope;
class Settings;
class CustomValueType;
class GenericStructValueType;
class NullValueType;

/**
 * @brief The Runtime class Loads LLVM-IR runtime from a bitcode file and creates the basic ValueTypes.
 */
class Runtime {
	public:
		static Runtime *instance();
		Runtime(ErrorHandler *errorHandler);
		~Runtime();
		/**
		 * @brief load Loads the runtime from a bitcode file.
		 * @param strPool A pointer to the global string pool.
		 * @param settings A pointer to the settings
		 * @return True, if loading succeeded, false otherwise
		 */
		bool load(StringPool *strPool, Settings *settings);
		llvm::Module *module() {return mModule;}
		std::vector<RuntimeFunction*> functions() const {return mFunctions;}
		llvm::Function *cbMain() const {return mCBMain;}
		llvm::Function *cbInitialize() const { return mCBInitialize; }

		StringValueType *stringValueType() const {return mStringValueType;}
		IntValueType *intValueType() const {return mIntValueType;}
		FloatValueType *floatValueType() const {return mFloatValueType;}
		ShortValueType *shortValueType() const {return mShortValueType;}
		ByteValueType *byteValueType() const {return mByteValueType;}
		BooleanValueType *booleanValueType() const {return mBooleanValueType;}
		TypePointerCommonValueType *typePointerCommonValueType() const {return mTypePointerCommonValueType;}
		TypeValueType *typeValueType() const { return mTypeValueType; }
		GenericArrayValueType *genericArrayValueType() const { return mGenericArrayValueType; }
		GenericStructValueType *genericStructValueType() const { return mGenericStructValueType; }
		NullValueType *nullValueType() const { return mNullValueType; }

		llvm::Function *allocatorFunction() const { return mAllocatorFunction; }
		llvm::Function *freeFunction() const { return mFreeFunction; }

		const llvm::DataLayout &dataLayout() const { return *mDataLayout; }
		llvm::Type *typeLLVMType() const { return mTypeLLVMType; }
		llvm::Type *typeMemberLLVMType() const { return mTypeMemberLLVMType; }
		llvm::PointerType *typeMemberPointerLLVMType() const { return mTypeMemberLLVMType->getPointerTo(); }

		const ValueTypeCollection &valueTypeCollection() const { return mValueTypeCollection; }
		ValueTypeCollection &valueTypeCollection() { return mValueTypeCollection; }
	private:
		bool loadRuntimeFunctions();
		bool loadDefaultRuntimeFunctions();
		bool loadValueTypes(StringPool *strPool);
		bool isAllocatorFunctionValid();
		bool isFreeFuntionValid();
		bool loadFunctionMapping(const std::string &functionMapping);
		bool loadCustomDataTypes(const std::string &customDataTypes);

		void error(int code, std::string msg, CodePoint cp);
		void warning(int code, std::string msg, CodePoint cp);


		bool mValid;
		llvm::Module *mModule;
		std::vector<RuntimeFunction*> mFunctions;
		llvm::Function *mCBMain;
		llvm::Function *mCBInitialize;

		IntValueType *mIntValueType;
		FloatValueType *mFloatValueType;
		StringValueType *mStringValueType;
		ShortValueType *mShortValueType;
		ByteValueType *mByteValueType;
		BooleanValueType *mBooleanValueType;
		TypeValueType *mTypeValueType;
		TypePointerCommonValueType *mTypePointerCommonValueType;
		GenericArrayValueType *mGenericArrayValueType;
		GenericStructValueType *mGenericStructValueType;
		NullValueType *mNullValueType;


		ValueTypeCollection mValueTypeCollection;

		llvm::DataLayout *mDataLayout;

		llvm::Function *mAllocatorFunction;
		llvm::Function *mFreeFunction;

		llvm::Type *mTypeLLVMType;
		llvm::Type *mTypeMemberLLVMType;
		llvm::Type *mGenericArrayLLVMType;
		llvm::Type *mGenericStructLLVMType;

		ErrorHandler *mErrorHandler;

		std::multimap<std::string, std::string> mFunctionMapping;
};

#endif // RUNTIME_H
