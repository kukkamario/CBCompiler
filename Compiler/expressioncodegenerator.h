#ifndef EXPRESSIONGENERATOR_H
#define EXPRESSIONGENERATOR_H
#include <QObject>
#include "valuetype.h"
#include "value.h"
#include "abstractsyntaxtree.h"
class Builder;
class Scope;
class QFile;
class ExpressionCodeGenerator : public QObject {
	Q_OBJECT
	public:
		ExpressionCodeGenerator(QObject *parent = 0);
		void setBuilder(Builder *b);
		void setScope(Scope *s);
		Value generate(ast::Node *n);
		Value generate(ast::FunctionCallOrArraySubscript *n);
		Value generate(ast::Float *n);
		Value generate(ast::Integer *n);
		Value generate(ast::String *n);
		Value generate(ast::Expression *n);
		Value generate(ast::Unary *n);
		Value generate(ast::Variable *n);
		Value generate(ast::TypePtrField *n);

		QList<Value> generateParameterList(const QList<ast::Node*> &params);
	private:
		Builder *mBuilder;
		Scope *mScope;
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // EXPRESSIONGENERATOR_H
