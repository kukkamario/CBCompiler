#ifndef RUNTIME_H
#define RUNTIME_H
#include <QFile>
#include "runtimefunction.h"
#include "llvm.h"
class Runtime : QObject
{
		Q_OBJECT
	public:
		Runtime();
		bool load(const QString &file);
		llvm::Module *module() {return mModule;}
		QList<RuntimeFunction*> functions();
	private:
		llvm::Module *mModule;
		QList<RuntimeFunction*> mFunctions;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // RUNTIME_H
