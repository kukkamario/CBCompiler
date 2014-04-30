#ifndef VALUETYPE_H
#define VALUETYPE_H
#include <QString>
#include <QObject>
#include "llvm.h"
class Value;
namespace llvm {
	class Value;
	class Type;
	class Module;
	class LLVMContext;
}
class Builder;
class Runtime;
class ValueType {
	public:
		enum CastCost {
			ccNoCost = 0,
			ccCastToBigger = 1,
			ccCastToSmaller = 2,
			ccNoCast = 100
		};

		ValueType(Runtime *r);
		ValueType(Runtime *r, llvm::Type *type);
		virtual ~ValueType() {}
		virtual QString name() const = 0;
		virtual bool isNamedValueType() const = 0;
		llvm::Type *llvmType() const {return mType;}
		virtual bool isTypePointer() const = 0;
		virtual bool isNumber() const = 0;
		virtual llvm::Constant *defaultValue() const = 0;

		/**
		 * @brief size Returns ValueType's size in bytes.
		 * @return Size of ValueType in bytes.
		 */
		virtual int size() const = 0;

		bool canBeCastedToValueType(ValueType *to) const;
		/** Calculates cost for casting this ValueType to given ValueType.
		 *If returned cost is ccNoCast, cast cannot be done. */
		virtual CastCost castingCostToOtherValueType(ValueType *to) const = 0;

		virtual Value cast(Builder *builder, const Value &v) const = 0;

		virtual ValueType *operatorResultType(int opType, ValueType *operand1, ValueType *operand2) const { return 0; }
		virtual ValueType *operatorResultType(int opType, ValueType *operand) const { return 0; }
		bool hasOperator(int opType, ValueType *operand1, ValueType *operand2) const { return operatorResultType(opType, operand1, operand2) != 0; }
		bool hasOperator(int opType, ValueType *operand) const { return operatorResultType(opType, operand) != 0; }
		virtual Value generateOperation(int opType, const Value &operand1, const Value &operand2) const {}
		virtual Value generateOperation(int opType, const Value &operand) const {}

		virtual Value member(Builder *builder, const Value &self, const QString &name) const { return Value(); }
		virtual ValueType *memberType(const QString &name) const { return 0; }
		bool hasMember(const QString &name) const { return memberType(name) != 0; }

		llvm::LLVMContext &context();
		Runtime *runtime() const { return mRuntime; }
	protected:
		llvm::Type *mType;
		Runtime *mRuntime;
};

#endif // VALUETYPE_H
