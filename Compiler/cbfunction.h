#ifndef CBFUNCTION_H
#define CBFUNCTION_H
#include "function.h"
#include "constantvalue.h"
#include <utility>
#include "codepoint.h"
#include "functionvaluetype.h"
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

		CBFunction(const std::string & name, ValueType *retValue, const std::vector<Parameter> &params, Scope *scope, const CodePoint &cp);
		~CBFunction();

		void generateFunction(Runtime *runtime);
		void setScope(Scope *scope);
		Scope *scope() const {return mScope;}
		bool isRuntimeFunction() const {return false;}
		Value call(Builder *builder, const std::vector<Value> &params);
		const std::vector<Parameter> &parameters() const { return mParams; }
		FunctionValueType *functionValueType() const { return mFunctionValueType; }
	private:
		std::vector<Parameter> mParams;
		Scope *mScope;
		FunctionValueType *mFunctionValueType;
};

#endif // CBFUNCTION_H
