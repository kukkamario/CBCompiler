#ifndef RUNTIME_H
#define RUNTIME_H
#include <QFile>
#include "runtimefunction.h"
#include "llvm.h"
#include "valuetype.h"
#include <QMap>
#include <QMultiMap>
#include <QHash>
#include "valuetypecollection.h"
#include "codepoint.h"

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
class GenericClassValueType;

/**
 * @brief The Runtime class Loads LLVM-IR runtime from a bitcode file and creates the basic ValueTypes.
 */
class Runtime : public QObject {
		Q_OBJECT
	public:
		static Runtime *instance();
		Runtime();
		~Runtime();
		/**
		 * @brief load Loads the runtime from a bitcode file.
		 * @param strPool A pointer to the global string pool.
		 * @param settings A pointer to the settings
		 * @return True, if loading succeeded, false otherwise
		 */
		bool load(StringPool *strPool, const Settings &settings);
		llvm::Module *module() {return mModule;}
		QList<RuntimeFunction*> functions() const {return mFunctions;}
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
		GenericClassValueType *genericClassValueType() const { return mGenericClassValueType; }

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
		bool loadFunctionMapping(const QString &functionMapping);
		bool loadCustomDataTypes(const QString &customDataTypes);

		bool mValid;
		llvm::Module *mModule;
		QList<RuntimeFunction*> mFunctions;
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
		GenericClassValueType *mGenericClassValueType;

		ValueTypeCollection mValueTypeCollection;

		llvm::DataLayout *mDataLayout;

		llvm::Function *mAllocatorFunction;
		llvm::Function *mFreeFunction;

		llvm::Type *mTypeLLVMType;
		llvm::Type *mTypeMemberLLVMType;
		llvm::Type *mGenericArrayLLVMType;
		llvm::Type *mGenericClassLLVMType;

		QMultiMap<QString, QString> mFunctionMapping;
	signals:
		void error(int code, QString msg, CodePoint cp);
		void warning(int code, QString msg, CodePoint cp);
};

#endif // RUNTIME_H
