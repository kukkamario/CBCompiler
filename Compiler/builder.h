#ifndef BUILDER_H
#define BUILDER_H
#include "value.h"
#include "llvm.h"
#include "function.h"
#include <QList>
#include <QStack>
#include "variablesymbol.h"
#include "runtime.h"



/**
 * @brief The Builder class. Helper class for llvm-IR generation.
 */

class Builder {
	public:
		Builder(llvm::LLVMContext &context);
		void setRuntime(Runtime *r);
		void setStringPool(StringPool *s);
		void setInsertPoint(llvm::BasicBlock *basicBlock);
		llvm::IRBuilder<> & irBuilder() { return mIRBuilder; }
		Value toInt(const Value &v);
		Value toFloat(const Value &v);
		Value toString(const Value &v);
		Value toShort(const Value &v);
		Value toByte(const Value &v);
		Value toBoolean(const Value &v);
		Value toValueType(ValueType *to, const Value &v);
		llvm::Value *llvmValue(const Value &v);
		llvm::Value *llvmValue(int i);
		llvm::Value *llvmValue(float i);
		llvm::Value *llvmValue(uint16_t i);
		llvm::Value *llvmValue(uint8_t i);
		llvm::Value *llvmValue(const QString &s);
		llvm::Value *llvmValue(bool t);
		llvm::Value *llvmValue(const ConstantValue &v);
		Value call(Function *func, QList<Value> &params);
		void branch(llvm::BasicBlock *dest);
		void branch(const Value &cond, llvm::BasicBlock *ifTrue, llvm::BasicBlock *ifFalse);

		void construct(VariableSymbol *var);
		void store(VariableSymbol *var, const Value &v);
		Value load(const VariableSymbol *var);
		void destruct(VariableSymbol *var);
		void destruct(const Value &a);

		llvm::GlobalVariable *createGlobalVariable(ValueType *type, bool isConstant, llvm::GlobalValue::LinkageTypes linkage, llvm::Constant *initializer, const llvm::Twine &name = llvm::Twine());
		llvm::GlobalVariable *createGlobalVariable(llvm::Type *type, bool isConstant, llvm::GlobalValue::LinkageTypes linkage, llvm::Constant *initializer, const llvm::Twine &name = llvm::Twine());
		Runtime *runtime()const{return mRuntime;}
		llvm::LLVMContext &context();

		/// Operators: the caller has to destroy parameters

		Value not_(const Value &a);
		Value minus(const Value &a);
		Value plus(const Value &a);
		Value add(const Value &a, const Value &b);
		Value subtract(const Value &a, const Value &b);
		Value multiply(const Value &a, const Value &b);
		Value divide(const Value &a, const Value &b);
		Value mod(const Value &a, const Value &b);
		Value shl(const Value &a, const Value &b);
		Value shr(const Value &a, const Value &b);
		Value sar(const Value &a, const Value &b);
		Value and_(const Value &a, const Value &b);
		Value or_(const Value &a, const Value &b);
		Value xor_(const Value &a, const Value &b);
		Value power(const Value &a, const Value &b);

		Value less(const Value &a, const Value &b);
		Value lessEqual(const Value &a, const Value &b);
		Value greater(const Value &a, const Value &b);
		Value greaterEqual(const Value &a, const Value &b);
		Value equal(const Value &a, const Value &b);
		Value notEqual(const Value &a, const Value &b);

		void pushInsertPoint();
		void popInsertPoint();
	private:
		llvm::IRBuilder<> mIRBuilder;
		QStack<llvm::IRBuilder<>::InsertPoint> mInsertPointStack;
		QList<llvm::Value*> mStringData;
		Runtime *mRuntime;
		StringPool *mStringPool;
};

#endif // BUILDER_H
