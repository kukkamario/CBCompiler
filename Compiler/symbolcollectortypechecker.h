#ifndef SYMBOLCOLLECTORTYPECHECKER_H
#define SYMBOLCOLLECTORTYPECHECKER_H
#include <QObject>
#include "abstractsyntaxtree.h"
class Scope;
class ValueType;
class Runtime;
class SymbolCollectorTypeChecker : public QObject {
		Q_OBJECT
	public:
		SymbolCollectorTypeChecker();
		void setRuntime(Runtime *r);
		bool run(const ast::Block *block, Scope *scope);
		void setForceVariableDeclaration(bool s) {mForceVariableDeclaration = s;}
	private:
		ValueType *typeCheck(ast::Node *s);
		ValueType *typeCheck(ast::Expression *s);
		ValueType *typeCheck(ast::Unary *s);
		ValueType *typeCheck(ast::New *s);
		ValueType *typeCheck(ast::FunctionCallOrArraySubscript *s);
		ValueType *typeCheck(ast::TypePtrField *s);
		ValueType *typeCheck(ast::Variable *s);

		bool checkStatement(ast::Node *s);
		bool checkStatement(ast::IfStatement *s);
		bool checkStatement(ast::WhileStatement *s);
		bool checkStatement(ast::AssignmentExpression *s);
		bool checkStatement(ast::ArraySubscriptAssignmentExpression *s);
		bool checkStatement(ast::ForToStatement *s);
		bool checkStatement(ast::ForEachStatement *s);
		bool checkStatement(ast::ArrayDefinition *s);
		bool checkStatement(ast::VariableDefinition *s);
		bool checkStatement(ast::SelectStatement *s);
		bool checkStatement(ast::RepeatForeverStatement *s);
		bool checkStatement(ast::RepeatUntilStatement *s);


		bool mForceVariableDeclaration;
		Runtime *mRuntime;
		int mLine;
		QFile *mFile;
		Scope *mScope;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // SYMBOLCOLLECTORTYPECHECKER_H
