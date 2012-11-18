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

};

#endif // CODEGENERATOR_H
