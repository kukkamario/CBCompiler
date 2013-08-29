#ifndef SYMBOLCOLLECTORTYPECHECKER_H
#define SYMBOLCOLLECTORTYPECHECKER_H
#include <QObject>
#include "abstractsyntaxtree.h"
#include <QMultiMap>
class Scope;
class ValueType;
class Runtime;
class VariableSymbol;
class ArraySymbol;
class ConstantExpressionEvaluator;
class CBFunction;

class SymbolCollectorTypeChecker : public QObject {
		Q_OBJECT
	public:
		SymbolCollectorTypeChecker();
		void setRuntime(Runtime *r) {mRuntime = r;}
		bool run(ast::Block *block, Scope *scope);
		CBFunction *checkFunctionDefinitionAndAddToGlobalScope(ast::FunctionDefinition *func, Scope *globalScope);
		void setForceVariableDeclaration(bool s) {mForceVariableDeclaration = s;}
		void setGlobalScope(Scope *s) {mGlobalScope = s;}
		void setScope(Scope *s) { mScope = s; }
		void setReturnValueType(ValueType *s) {mReturnValueType = s;}
		void setFile(const QString &file) { mFile = file; }
		void setLine(int line) { mLine = line; }
		void setConstantExpressionEvaluator(ConstantExpressionEvaluator *constEval);
		VariableSymbol *declareVariable(const ast::Variable *var);
	private:
		struct CodeLineInfo {
				CodeLineInfo() : mLine(0) {}
				CodeLineInfo(int line, const QString &f) : mLine(line), mFile(f) {}
				int mLine;
				QString mFile;
		};

		ValueType *typeCheckExpression(ast::Node *s);
		ValueType *typeCheck(ast::Node *s);
		ValueType *typeCheck(ast::Expression *s);
		ValueType *typeCheck(ast::Unary *s);
		ValueType *typeCheck(ast::FunctionCallOrArraySubscript *s);
		ValueType *typeCheck(ast::SpecialFunctionCall *s);
		ValueType *typeCheck(ast::TypePtrField *s);
		ValueType *typeCheck(ast::Variable *s);

		bool checkBlock(ast::Block *block);
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
		bool checkStatement(ast::SpecialFunctionCall *s);
		bool checkStatement(ast::Return *s);
		bool checkStatement(ast::Label *s);
		bool checkStatement(ast::Goto *s);
		bool checkStatement(ast::Gosub *s);
		bool checkStatement(ast::Exit *s);
		bool checkStatement(ast::CommandCallOrArraySubscriptAssignmentExpression *s);

		ArraySymbol *findAndValidateArraySymbol(const QString &name);
		ValueType *findValueType(const QString &name, int line, const QString &file);
		bool validateArrayIndex(ArraySymbol *array, const QList<ast::Node*> &index);

		ValueType *checkTypePointerType(const QString &typeName);

		bool tryCastToBoolean(ValueType *t);

		ValueType *checkVariable(const QString &name, const QString &type);
		void replaceParentBlockNode(ast::Node *search, ast::Node *replace);

		bool mForceVariableDeclaration;
		Runtime *mRuntime;
		int mLine;
		QString mFile;
		Scope *mScope;
		Scope *mGlobalScope;
		ValueType *mReturnValueType;
		int mExitLevel;
		QMultiMap<QString, CodeLineInfo> mRequiredLabels;
		bool insideExpression() { return mExpressionLevel > 0; }
		int mExpressionLevel;
		ast::Block *mParentBlock;
		ConstantExpressionEvaluator *mConstEval;
	signals:
		void error(int code, QString msg, int line, const QString &file);
		void warning(int code, QString msg, int line, const QString &file);
};

#endif // SYMBOLCOLLECTORTYPECHECKER_H
