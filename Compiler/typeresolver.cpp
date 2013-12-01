#include "typeresolver.h"

TypeResolver::TypeResolver(Runtime *runtime)
{
}


ValueType *TypeResolver::resolve(ast::Node *type) {
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

ValueType *TypeResolver::resolve(ast::ArrayType *arrTy)
{
}

ValueType *TypeResolver::resolve(ast::DefaultType *defaultTy)
{
}

ValueType *TypeResolver::resolve(ast::NamedType *namedTy)
{
}

ValueType *TypeResolver::resolve(ast::BasicType *basicTy)
{
}


