#ifndef RUNTIME_H
#define RUNTIME_H
#include <QFile>
#include "runtimefunction.h"
#include "llvm.h"
#include "valuetype.h"
class Runtime : QObject {
		Q_OBJECT
	public:
		Runtime();
		bool load(const QString &file);
		llvm::Module *module() {return mModule;}
		QList<RuntimeFunction*> functions();
		QList<ValueType*> valueTypes() const {return mValueTypes;}
		llvm::Function *cbMain() {return mCBMain;}
	private:
		bool loadValueTypes();
		llvm::Module *mModule;
		QList<RuntimeFunction*> mFunctions;
		QList<ValueType*> mValueTypes;
		llvm::Function *mCBMain;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // RUNTIME_H
