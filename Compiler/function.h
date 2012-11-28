#ifndef FUNCTION_H
#define FUNCTION_H
#include <QList>
#include <QString>
class ValueType;
class QFile;
namespace llvm {
	class Function;
}
class Function {
	public:
		typedef QList<ValueType*> ParamList;
		Function(QFile *f, int line);
		QString name() const{return mName;}
		ValueType *returnValue() {return mReturnValue;}
		int requiredParams() const { return mRequiredParams;} //Params without default value
		ParamList paramTypes() const {return mParamTypes;}
		llvm::Function *function()const{return mFunction;}
	protected:
		QString mName;
		ValueType *mReturnValue;
		ParamList mParamTypes;
		int mRequiredParams;
		llvm::Function *mFunction;
		QFile *mFile;
		int mLine;
};

#endif // FUNCTION_H
