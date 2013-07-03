#ifndef VALUETYPE_H
#define VALUETYPE_H
#include <QString>
#include <QObject>
#include <llvm/IRBuilder.h>
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
		enum Type {
			Invalid,
			Integer,
			Float,
			String,
			Short,
			Byte,
			Boolean,
			TypePointer,
			NULLTypePointer
		};

		typedef unsigned int CastCostType;
		static const CastCostType maxCastCost = 10000;

		ValueType(Runtime *r);
		~ValueType() {}
		virtual QString name() const = 0;
		llvm::Type *llvmType() {return mType;}
		virtual Type type() const = 0;
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
		 *If returned cost is over or equal to maxCastCost, cast cannot be done. */
		virtual CastCostType castingCostToOtherValueType(ValueType *to) const = 0;
		virtual Value cast(Builder *builder, const Value &v) const = 0;
		llvm::LLVMContext &context();
	protected:
		llvm::Type *mType;
		Runtime *mRuntime;
};

#endif // VALUETYPE_H
