#ifndef CONVERSIONHELPER_H
#define CONVERSIONHELPER_H
#include "abstractsyntaxtree.h"
#include "valuetype.h"

inline ValueType::eType valueTypeFromVarType(ast::Variable::VarType t) {
	switch (t) {
		case ast::Variable::Default:
		case ast::Variable::Integer:
			return ValueType::Integer;
		case ast::Variable::Float:
			return ValueType::Float;
		case ast::Variable::Short:
			return ValueType::Short;
		case ast::Variable::Byte:
			return ValueType::Byte;
		case ast::Variable::String:
			return ValueType::String;
		case ast::Variable::TypePtr:
			return ValueType::TypePointer;
		default:
			return ValueType::Invalid;
	}
}
#endif // CONVERSIONHELPER_H
