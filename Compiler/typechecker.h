#ifndef TYPECHECKER_H
#define TYPECHECKER_H
#include <QObject>
#include "astvisitor.h"

class Scope;
class ValueType;

class TypeChecker : public QObject, protected ast::Visitor  {
		Q_OBJECT
	public:
		TypeChecker();
		bool check(ast::Program *program, Scope *globalScope, Scope *mainScope);
	private:
		void visit(ast::Expression *expr);
		void visit(ast::ArrayInitialization *arrayInit);
		void visit(ast::Goto *g);
		void visit(ast::Gosub *g);
		void visit(ast::Dim *d);
		void visit(ast::Global *g);

		ValueType *check(ast::Expression *expr);
		ValueType *check(ast::FunctionCall *func);
		ValueType *check(ast::ArraySubscript *arraySubscript);



		Scope *mGlobalScope;
		Scope *mCurrentScope;
		Scope *mMainScope;

private slots:
	void errorOccured(int, QString, CodePoint);
signals:
	void warning(int code, QString msg, CodePoint codePoint);
	void error(int code, QString msg, CodePoint codePoint);
};

#endif // TYPECHECKER_H
