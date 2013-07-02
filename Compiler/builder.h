#ifndef BUILDER_H
#define BUILDER_H
#include "value.h"
#include "llvm.h"
#include "function.h"
#include <QList>
#include <QStack>

#include "runtime.h"
class ArraySymbol;
class VariableSymbol;

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

		void initilizeArray(ArraySymbol *array, const QList<Value> &dimSizes);
		void arraySubscriptStore(ArraySymbol *array, const QList<Value> &dims, const Value &val);
		Value arraySubscriptLoad(ArraySymbol *array, const QList<Value> &dims);
		llvm::Value *calculateArrayElementCount(const QList<Value> &dimSizes);
		llvm::Value *calculateArrayMemorySize(ArraySymbol *array, const QList<Value> &dimSizes);
		/**
		 * @brief arrayElement calculates a pointer address of the array element.
		 * @param array
		 * @param index
		 * @return A pointer to an array element in index
		 */
		llvm::Value *arrayElementPointer(ArraySymbol *array, const QList<Value> &index);
		llvm::Value *arrayIndexMultiplier(ArraySymbol *array, int index);
		void fillArrayIndexMultiplierArray(ArraySymbol *array, const QList<Value> &dimSizes);

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

		/**
		 * @brief Copies the values of num bytes from the location pointed by src directly to the memory block pointed by dest.
		 * @param src Source pointer, a pointer type
		 * @param dest Destination pointer, a pointer type
		 * @param num i32
		 * @param align The memory alignment
		 */
		void memCopy(llvm::Value *src, llvm::Value *dest, llvm::Value *num, int align = 0);
		/**
		 * @brief Sets the first num bytes of the block of memory pointed by ptr to the specified value (i8)
		 * @param ptr
		 * @param num
		 * @param value
		 * @param align The memory alignment
		 */
		void memSet(llvm::Value *ptr, llvm::Value *num, llvm::Value *value, int align = 0);

		/**
		 * @brief Allocates size bytes dynamically.
		 * @param size Number of the bytes to be allocated. i32
		 * @return i8 pointer to allocated memory
		 */
		llvm::Value *allocate(llvm::Value *size);

		/**
		 * @brief Frees memory allocated with allocate.
		 * @param val A pointer to the memory. Doesn't have to be any spesific pointer type.
		 */
		void free(llvm::Value *ptr);

		/**
		 * @brief pointerToBytePointer casts val to a byte pointer using a bitcast.
		 * @param val Value to be casted
		 * @return i8 pointer
		 */
		llvm::Value *pointerToBytePointer(llvm::Value *ptr);

		//Dont work. Dont use
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
