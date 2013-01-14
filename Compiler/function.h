#ifndef FUNCTION_H
#define FUNCTION_H
#include <QList>
#include <QString>
class ValueType;
class QFile;
class Builder;
class Value;
namespace llvm {
	class Function;
}

class Function {
	public:
		typedef QList<ValueType*> ParamList;
		Function(const QString &name, QFile *f, int line);
		QString name() const{return mName;}
		ValueType *returnValue() {return mReturnValue;}
		int requiredParams() const { return mRequiredParams;} //Params without default value
		const ParamList &paramTypes() const {return mParamTypes;}
		llvm::Function *function()const{return mFunction;}
		QFile *file()const {return mFile;}
		int line()const{return mLine;}
		virtual bool isRuntimeFunction() const = 0;
		bool isCommand() const {return mReturnValue == 0;}
		virtual Value call(Builder *builder, const QList<Value> &params) = 0;
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
