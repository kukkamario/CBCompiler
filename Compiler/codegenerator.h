#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include "scope.h"
#include "abstractsyntaxtree.h"
#include "runtime.h"
#include "stringpool.h"
class CodeGenerator : public QObject{
		Q_OBJECT
	public:
		CodeGenerator();
		bool initialize(const QString &runtimeFile);
		bool generate(ast::Program *program);
	private:
		Runtime mRuntime;
		StringPool mStringPool;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // CODEGENERATOR_H
