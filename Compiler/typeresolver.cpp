#include "typeresolver.h"

TypeResolver::TypeResolver(Runtime *runtime) :
	mRuntime(runtime) {
}


ValueType *TypeResolver::resolve(ast::Node *type) {
	mRuntime
	switch(type->type()) {
		case ast::Node::ntDefaultType:
			return resolve(static_cast<ast::DefaultType*>(type));
		case ast::Node::ntDefaultType:
			return resolve(static_cast<ast::DefaultType*>(type));
		case ast::Node::ntDefaultType:
			return resolve(static_cast<ast::DefaultType*>(type));
		case ast::Node::ntDefaultType:
			return resolve(static_cast<ast::DefaultType*>(type));
	}
}

QString *TypeResolver::resolve(ast::ArrayType *arrTy) {

}

ValueType *TypeResolver::resolve(ast::DefaultType *defaultTy)
{
}

ValueType *TypeResolver::resolve(ast::NamedType *namedTy)
{
}

QString TypeResolver::resolve(ast::BasicType *basicTy)
{
}


