#ifndef VALUETYPE_H
#define VALUETYPE_H
#include <QString>
#include <QObject>
#include "llvm.h"
#include "operationflags.h"
#include "value.h"
namespace llvm {
	class Value;
	class Type;
	class Module;
	class LLVMContext;
}
class Builder;
class Runtime;
class Function;
class ValueType {
	public:
		enum CastCost {
			ccNoCost = 0,
			ccCastToBoolean,
			ccCastToBigger,
			ccCastToSmaller,
			ccCastToString,
			ccCastFromString,

			//Must be the last
			ccNoCast
		};

		/* Keep this priority order */
		enum BasicType {
			Boolean = 1,
			Byte,
			Short,
			Integer,
			Float,
			String,
			Unknown
		};

		ValueType(Runtime *r);
		ValueType(Runtime *r, llvm::Type *type);
		virtual ~ValueType() {}
		virtual QString name() const = 0;
		virtual bool isNamedValueType() const = 0;
		llvm::Type *llvmType() const {return mType;}
		virtual bool isTypePointer() const = 0;
		virtual bool isArray() const { return false; }
		virtual bool isNumber() const = 0;
		virtual bool isFunctionSelector() const { return false; }
		virtual bool isStruct() const { return false; }
		virtual llvm::Constant *defaultValue() const = 0;

		virtual BasicType basicType() const { return Unknown; }
		bool isBasicType() const { return basicType() != Unknown; }
		/**
		 * @brief size Returns ValueType's size in bytes.
		 * @return Size of ValueType in bytes.
		 */
		virtual int size() const = 0;

		bool canBeCastedToValueType(ValueType *to) const;
		/** Calculates cost for casting this ValueType to given ValueType.
		 *If returned cost is ccNoCast, cast cannot be done. */
		virtual CastCost castingCostToOtherValueType(const ValueType *to) const = 0;

		virtual Value cast(Builder *builder, const Value &v) const = 0;

		virtual Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		virtual Value generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const;
		virtual void generateDestructor(Builder *builder, const Value &value);
		virtual Value generateLoad(Builder *builder, const Value &var) const;

		virtual Value member(Builder *, const Value &, const QString &) const { return Value(); }
		virtual ValueType *memberType(const QString &) const { return 0; }
		bool hasMember(const QString &name) const { return memberType(name) != 0; }
		
		virtual bool isCallable() const { return false; }
		virtual QList<Function*> overloads() const { return QList<Function*>(); }

		llvm::LLVMContext &context();
		Runtime *runtime() const { return mRuntime; }

		static OperationFlags castCostOperationFlags(CastCost cc);
		static CastCost castToSameType(Builder *builder, Value &a, Value &b);

	protected:
		Value generateBasicTypeOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		Value generateBasicTypeOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const;

		llvm::Type *mType;
		Runtime *mRuntime;
};

#endif // VALUETYPE_H
