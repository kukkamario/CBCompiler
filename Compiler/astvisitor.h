#ifndef ASTVISITOR_H
#define ASTVISITOR_H
#include "abstractsyntaxtree.h"
namespace ast {
enum Type {
	ntBlock = 0,
	ntInteger,
	ntFloat,
	ntString,
	ntIdentifier,
	ntType,
	ntLabel,
	ntList,
	ntGoto,
	ntGosub,
	ntReturn,
	ntExit,

	ntDefaultType,
	ntBasicType,
	ntNamedType,
	ntArrayType,

	ntExpression,
	ntExpressionNode,
	ntUnary,
	ntArraySubscript,
	ntFunctionCall,
	ntDefaultValue,
	ntVariable,

	ntIfStatement,
	ntWhileStatement,
	ntRepeatForeverStatement,
	ntRepeatUntilStatement,
	ntForToStatement,
	ntForEachStatement,
	ntSelectStatement,
	ntCase,
	ntDefault,

	ntConst,
	ntDim,
	ntGlobal,
	ntVariableDefinition,
	ntArrayInitialization,
	ntFunctionDefinition,
	ntTypeDefinition,

	ntProgram,




	//Last
	ntInvalid
};
class AstVisitor {
	public:
		AstVisitor();

		virtual void visit(ast::Node *n) {



		}

		bool actBefore(ast::Node *n) { }
		bool actAfter(ast::Node *n)
};

}
#endif // ASTVISITOR_H
