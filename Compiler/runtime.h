#ifndef RUNTIME_H
#define RUNTIME_H
#include <QFile>
#include "runtimefunction.h"
#include "llvm.h"
#include "valuetype.h"
class IntValueType;
class StringValueType;
class FloatValueType;
class ShortValueType;
class ByteValueType;
class BooleanValueType;
class StringPool;
class Runtime : public QObject {
		Q_OBJECT
	public:
		static Runtime *instance();
		Runtime();
		~Runtime();
		bool load(StringPool *strPool, const QString &file);
		llvm::Module *module() {return mModule;}
		QList<RuntimeFunction*> functions();
		QList<ValueType*> valueTypes() const {return mValueTypes;}
		llvm::Function *cbMain() {return mCBMain;}
		StringValueType *stringValueType() {return mStringValueType;}
		IntValueType *intValueType() {return mIntValueType;}
		FloatValueType *floatValueType() {return mFloatValueType;}
		ShortValueType *shortValueType() {return mShortValueType;}
		ByteValueType *byteValueType() {return mByteValueType;}
		BooleanValueType *booleanValueType() {return mBooleanValueType;}
	private:
		void addRuntimeFunction(llvm::Function *func, const QString &name);

		bool loadValueTypes(StringPool *strPool);
		llvm::Module *mModule;
		QList<RuntimeFunction*> mFunctions;
		QList<ValueType*> mValueTypes;
		llvm::Function *mCBMain;
		IntValueType *mIntValueType;
		FloatValueType *mFloatValueType;
		StringValueType *mStringValueType;
		ShortValueType *mShortValueType;
		ByteValueType *mByteValueType;
		BooleanValueType *mBooleanValueType;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // RUNTIME_H
