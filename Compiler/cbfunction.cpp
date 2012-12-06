#include "cbfunction.h"
#include "scope.h"
#include "variablesymbol.h"

CBFunction::CBFunction(const QString &name, ValueType *retValue, const QList<CBFunction::Parametre> &params, int line, QFile *file):
	Function(name, file, line),
	mParams(params),
	mScope(new Scope(name)),
	mBlock(0){

	mReturnValue = retValue;
	for (QList<Parametre>::ConstIterator i = mParams.begin(), end = mParams.end(); i != end; ++i) {
		mParamTypes.append(i->mType);

		//Parametre variable symbols
		mScope->addSymbol(new VariableSymbol(i->mName, i->mType, mFile, mLine));
	}

}

bool CBFunction::generateFunction(const ast::FunctionDefinition *funcDef) {
	return true;
}
