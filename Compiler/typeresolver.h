#ifndef TYPERESOLVER_H
#define TYPERESOLVER_H
#include "abstractsyntaxtree.h"
#include <QObject>

class ValueType;
class Runtime;


class TypeResolver : public QObject{
		Q_OBJECT
	public:
		TypeResolver(Runtime *runtime);
		ValueType *resolve(ast::Node *type);
	signals:
		void error(int code, QString msg, CodePoint cp);
		void warning(int code, QString msg, CodePoint cp);

	private:
		Runtime *mRuntime;

		ValueType *resolve(ast::ArrayType *arrTy);
		ValueType *resolve(ast::DefaultType *defaultTy);
		ValueType *resolve(ast::NamedType *namedTy);
		ValueType *resolve(ast::BasicType *basicTy);
};

#endif // TYPERESOLVER_H
