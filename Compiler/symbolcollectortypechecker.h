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
		void setRuntime(Runtime *r) {mRuntime = r;}
		bool run(const ast::Block *block, Scope *scope);
		void setForceVariableDeclaration(bool s) {mForceVariableDeclaration = s;}
		void setGlobalScope(Scope *s) {mGlobalScope = s;}
		void setReturnValueType(ValueType *s) {mReturnValueType = s;}
	private:
		ValueType *typeCheck(ast::Node *s);
		ValueType *typeCheck(ast::Expression *s);
		ValueType *typeCheck(ast::Unary *s);
		ValueType *typeCheck(ast::New *s);
		ValueType *typeCheck(ast::FunctionCallOrArraySubscript *s);
		ValueType *typeCheck(ast::TypePtrField *s);
		ValueType *typeCheck(ast::Variable *s);

		bool checkBlock(const ast::Block *block);
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
		bool checkStatement(ast::CommandCall *s);
		bool checkStatement(ast::FunctionCallOrArraySubscript *s);
		bool checkStatement(ast::Return *s);

		ValueType *checkTypePointerType(const QString &typeName);

		bool tryCastToBoolean(ValueType *t);

		ValueType *checkVariable(const QString &name, ast::Variable::VarType type, const QString &typeName = QString());

		bool mForceVariableDeclaration;
		Runtime *mRuntime;
		int mLine;
		QFile *mFile;
		Scope *mScope;
		Scope *mGlobalScope;
		ValueType *mReturnValueType;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // SYMBOLCOLLECTORTYPECHECKER_H
