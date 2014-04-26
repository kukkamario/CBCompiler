#ifndef CBFUNCTION_H
#define CBFUNCTION_H
#include "function.h"
#include "constantvalue.h"
#include <QPair>
#include "codepoint.h"

class Scope;
class VariableSymbol;

/**
 * @brief The CBFunction class
 */

class CBFunction : public Function {
	public:
		struct Parameter {
				Parameter() : mVariableSymbol(0) {}
				VariableSymbol *mVariableSymbol;
				ConstantValue mDefaultValue;
		};

		CBFunction(const QString & name, ValueType *retValue, const QList<Parameter> &params, Scope *scope, const CodePoint &cp);
		void generateFunction(Runtime *runtime);
		void setScope(Scope *scope);
		Scope *scope() const {return mScope;}
		bool isRuntimeFunction() const {return false;}
		Value call(Builder *builder, const QList<Value> &params);
		const QList<Parameter> &parameters() const { return mParams; }
	private:
		QList<Parameter> mParams;
		Scope *mScope;
};

#endif // CBFUNCTION_H
