#include "typeresolver.h"
#include "errorcodes.h"
#include "runtime.h"
#include "intvaluetype.h"
#include "floatvaluetype.h"
#include "arrayvaluetype.h"
#include "stringvaluetype.h"

TypeResolver::TypeResolver(Runtime *runtime) :
	mRuntime(runtime) {
}


ValueType *TypeResolver::resolve(ast::Node *type) {
	switch(type->type()) {
		case ast::Node::ntDefaultType:
			return resolve(static_cast<ast::DefaultType*>(type));
		case ast::Node::ntBasicType:
			return resolve(static_cast<ast::BasicType*>(type));
		case ast::Node::ntNamedType:
			return resolve(static_cast<ast::NamedType*>(type));
		case ast::Node::ntArrayType:
			return resolve(static_cast<ast::ArrayType*>(type));
		default:
			assert(0 && "ast::Node is not a type node");
			return 0;
	}
}


ValueType *TypeResolver::resolve(ast::BasicType *basicType) {
	switch (basicType->valueType()) {
		case ast::BasicType::Integer:
			return mRuntime->intValueType();
		case ast::BasicType::Float:
			return mRuntime->floatValueType();
		case ast::BasicType::String:
			return mRuntime->stringValueType();
		default:
			assert(0 && "Invalid ast::BasicType::ValueType");
			return 0;
	}
}

ValueType *TypeResolver::resolve(ast::NamedType *namedType) {
	ValueType *ty = mRuntime->valueTypeCollection().findNamedType(namedType->identifier()->name());
	if (!ty) {
		emit error(ErrorCodes::ecSymbolNotValueType, tr("Symbol \"%1\" isn't a value type").arg(namedType->identifier()->name()), namedType->codePoint());
		return 0;
	}
	return ty;
}

ValueType *TypeResolver::resolve(ast::ArrayType *arrayType) {
	ValueType *parentType = resolve(arrayType->parentType());
	if (!parentType) return 0;
	return mRuntime->valueTypeCollection().arrayValueType(parentType, arrayType->dimensions());
}

ValueType *TypeResolver::resolve(ast::DefaultType *) {
	return mRuntime->intValueType();
}




