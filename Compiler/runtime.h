#ifndef RUNTIME_H
#define RUNTIME_H
#include <QFile>
#include "runtimefunction.h"
#include "llvm.h"
#include "valuetype.h"
#include <QMap>
class IntValueType;
class StringValueType;
class FloatValueType;
class ShortValueType;
class ByteValueType;
class BooleanValueType;
class StringPool;
class NullTypePointerValueType;
class Runtime : public QObject {
		Q_OBJECT
	public:
		static Runtime *instance();
		Runtime();
		~Runtime();
		bool load(StringPool *strPool, const QString &file);
		llvm::Module *module() {return mModule;}
		QList<RuntimeFunction*> functions() {return mFunctions;}
		QList<ValueType*> valueTypes() const {return mValueTypes;}
		llvm::Function *cbMain() {return mCBMain;}
		StringValueType *stringValueType() {return mStringValueType;}
		IntValueType *intValueType() {return mIntValueType;}
		FloatValueType *floatValueType() {return mFloatValueType;}
		ShortValueType *shortValueType() {return mShortValueType;}
		ByteValueType *byteValueType() {return mByteValueType;}
		BooleanValueType *booleanValueType() {return mBooleanValueType;}
		NullTypePointerValueType *nullTypePointerValueType() {return mNullTypePointerValueType;}
		ValueType *findValueType(ValueType::Type valType);
	private:
		void addRuntimeFunction(llvm::Function *func, const QString &name);
		bool loadValueTypes(StringPool *strPool);

		bool mValid;
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
		NullTypePointerValueType *mNullTypePointerValueType;
		QMap<ValueType::Type, ValueType *> mValueTypeEnum;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // RUNTIME_H
