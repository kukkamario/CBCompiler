#ifndef VALUETYPE_H
#define VALUETYPE_H
#include <QString>
#include <QObject>
class Value;
namespace llvm {
	class Value;
	class Type;
	class Module;
	class LLVMContext;
	namespace ____dumb__ { class DumbClass;}
	template<bool preserveNames = true, typename T = ____dumb__::DumbClass,typename Inserter = ____dumb__::DumpClass>
	class IRBuilder;

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
		virtual QString name() const = 0;
		llvm::Type *llvmType() {return mType;}
		virtual Type type() const = 0;
		virtual bool isTypePointer() const = 0;
		virtual bool isNumber() const = 0;
		/** Calculates cost for casting this ValueType to given ValueType.
		 *If returned cost is over maxCastCost, cast cannot be done. */
		virtual CastCostType castCost(ValueType *to) const = 0;
		virtual Value cast(Builder *builder, const Value &v) const = 0;
		llvm::LLVMContext &context();
	protected:
		llvm::Type *mType;
		Runtime *mRuntime;
};

#endif // VALUETYPE_H
