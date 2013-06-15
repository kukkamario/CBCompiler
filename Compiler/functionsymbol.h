#ifndef FUNCTIONSYMBOL_H
#define FUNCTIONSYMBOL_H
#include "symbol.h"
#include "function.h"
#include <QList>

/**
 * @brief The FunctionSymbol class contains one or more function/command overloads with the same name. It can search best overload for given parameters.
 */
class FunctionSymbol:public Symbol {
	public:
		FunctionSymbol(const QString &name);
		Type type() const{ return stFunctionOrCommand; }
		void addFunction(Function *func);
		Function *exactMatch(const Function::ParamList &params) const;
		/**
		 * @brief functions
		 * @return List of the function overloads
		 */
		QList<Function*> functions() const { return mFunctions;}

		/**
		 * @brief findBestOverload Searches best match for a given parameter list.
		 * @param paramTypes List of the parameter types.
		 * @param command Specificates whether searching a function (true) or a command (false). True is default.
		 * @param err If a valid pointer is given, err is set to match the result of the search.
		 * @return Pointer to the function overload that matches the parameters best. If error occurs, will return a null pointer.
		 */
		Function *findBestOverload(const QList<ValueType*> &paramTypes, bool command = false, OverloadSearchError *err = 0);
		QString info() const;
	private:
		QList<Function*> mFunctions;

};

#endif // FUNCTIONSYMBOL_H
