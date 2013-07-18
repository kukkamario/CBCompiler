#ifndef RUNTIME_H
#define RUNTIME_H
#include <QFile>
#include "runtimefunction.h"
#include "llvm.h"
#include "valuetype.h"
#include <QMap>
class IntValueType;
class StringValueType;
class FloatValueType;
class ShortValueType;
class ByteValueType;
class BooleanValueType;
class StringPool;
class TypePointerCommonValueType;
class TypeValueType;

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
		 * @param strPool Pointer to the global string pool.
		 * @param file Path to the runtime bitcode file
		 * @return True, if loading succeeded, false otherwise
		 */
		bool load(StringPool *strPool, const QString &file);
		llvm::Module *module() {return mModule;}
		QList<RuntimeFunction*> functions() {return mFunctions;}
		QList<ValueType*> valueTypes() const {return mValueTypes;}
		llvm::Function *cbMain() {return mCBMain;}
		llvm::Function *cbInitialize() { return mCBInitialize; }

		StringValueType *stringValueType() {return mStringValueType;}
		IntValueType *intValueType() {return mIntValueType;}
		FloatValueType *floatValueType() {return mFloatValueType;}
		ShortValueType *shortValueType() {return mShortValueType;}
		ByteValueType *byteValueType() {return mByteValueType;}
		BooleanValueType *booleanValueType() {return mBooleanValueType;}
		TypePointerCommonValueType *typePointerCommonValueType() {return mTypePointerCommonValueType;}
		TypeValueType *typeValueType() { return mTypeValueType; }

		ValueType *findValueType(ValueType::eType valType);

		llvm::Function *allocatorFunction() const { return mAllocatorFunction; }
		llvm::Function *freeFunction() const { return mFreeFunction; }

		const llvm::DataLayout &dataLayout() const { return *mDataLayout; }
		llvm::Type *typeLLVMType() const { return mTypeLLVMType; }
		llvm::Type *typeMemberLLVMType() const { return mTypeMemberLLVMType; }
		llvm::PointerType *typeMemberPointerLLVMType() const { return mTypeMemberLLVMType->getPointerTo(); }


	private:
		void addRuntimeFunction(llvm::Function *func, const QString &name);
		void addDefaultRuntimeFunction(llvm::Function *func, const QString &name);
		bool loadValueTypes(StringPool *strPool);
		bool isAllocatorFunctionValid();
		bool isFreeFuntionValid();

		bool mValid;
		llvm::Module *mModule;
		QList<RuntimeFunction*> mFunctions;
		QList<ValueType*> mValueTypes;
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
		QMap<ValueType::eType, ValueType *> mValueTypeEnum;

		llvm::DataLayout *mDataLayout;

		llvm::Function *mAllocatorFunction;
		llvm::Function *mFreeFunction;

		llvm::Type *mTypeLLVMType;
		llvm::Type *mTypeMemberLLVMType;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // RUNTIME_H
