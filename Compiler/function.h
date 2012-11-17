#ifndef FUNCTION_H
#define FUNCTION_H
#include <QList>
class ValueType;
class Function {
	public:
		typedef QList<ValueType*> ParamList;
		Function();
		ValueType *returnValue() {return mReturnValue;}
		int requiredParams() const { return mRequiredParams;} //Params without default value
		ParamList paramTypes() const {return mParamTypes;}
	protected:
		ValueType *mReturnValue;
		ParamList mParamTypes;
		int mRequiredParams;
};

#endif // FUNCTION_H
