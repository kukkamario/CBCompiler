#include "parser.h"
#include "errorcodes.h"
#include "warningcodes.h"
#include <assert.h>
#include <boost/format.hpp>

Parser::Parser(ErrorHandler *errHandler):
	mStatus(Ok),
	mErrorHandler(errHandler) {
}

Parser::~Parser() {

}

typedef ast::Node *(Parser::*BlockParserFunction)(Parser::TokIterator &);
static BlockParserFunction blockParsers[] =  {
	&Parser::tryGotoGosubAndLabel,
	&Parser::tryDim,
	&Parser::tryGlobalDefinition,
	&Parser::tryRedim,
	&Parser::tryIfStatement,
	&Parser::tryWhileStatement,
	&Parser::tryRepeatStatement,
	&Parser::tryForStatement,
	&Parser::tryReturn,
	&Parser::trySelectStatement,
	&Parser::tryConstDefinition,
	&Parser::tryExit,
	&Parser::tryAssignmentExpression,
	&Parser::tryDelete
};
static const int blockParserCount = 14;

ast::Node *Parser::expectBlock(Parser::TokIterator &i) {
	std::vector<ast::Node*> statements;
	CodePoint startCp = i->codePoint();
	while (true) {
		if (i->isEndOfStatement()) {
			i++;
			if (i->type() == Token::EndOfTokens) {
				i--;
				ast::Block *block = new ast::Block(startCp, i->codePoint());
				block->setChildNodes(statements);
				return block;
			}
			continue;
		}
		ast::Node * n = 0;
		for (int p = 0; p < blockParserCount; p++) {
			n =	(this->*(blockParsers[p]))(i);
			if (mStatus == Error) return 0;
			if (n) {
				expectEndOfStatement(i);
				if (mStatus == Error) return 0;
				break;
			}
		}
		if (n) {
			statements.push_back(n);
		}
		else {
			break;
		}
	}
	ast::Block *block = new ast::Block(startCp, i->codePoint());
	block->setChildNodes(statements);
	return block;
}

ast::Node *Parser::expectInlineBlock(Parser::TokIterator &i) {
	std::vector<ast::Node*> nodes;
	CodePoint startCp = i->codePoint();
	while (i->line() == startCp.line()) {
		if (i->isEndOfStatement()) {
			i++;
			if (i->type() == Token::EndOfTokens) {
				i--;
				break;
			}
			continue;
		}
		ast::Node * n;
		for (int p = 0; p < blockParserCount; p++) {
			n = (this->*(blockParsers[p]))(i);
			if (mStatus == Error) return 0;
			if (n) {
				if (i->type() == Token::EOL || i->type() == Token::kElse || i->type() == Token::kElseIf) { // WTF CB?
					nodes.push_back(n);
					ast::Block *block = new ast::Block(startCp, i->codePoint());
					block->setChildNodes(nodes);
					return block;
				}
				else {
					expectEndOfStatement(i);
					if (mStatus == Error) return 0;
				}
				break;
			}
		}
		if (n) {
			nodes.push_back(n);
		}
		else {
			break;
		}
	}
	ast::Block *block = new ast::Block(startCp, i->codePoint());
	block->setChildNodes(nodes);
	return block;
}

ast::Node *Parser::tryAssignmentExpression(Parser::TokIterator &i) {
	switch (i->type()) {
		case Token::Identifier: {
			Parser::TokIterator begin = i;
			ast::Variable *var = tryVariable(i);
			if (var->valueType()->type() == ast::Node::ntDefaultType && (i->type() < Token::OperatorsBegin || i->type() > Token::OperatorsEnd || i->type() == Token::opNot || i->type() == Token::opMinus || i->type() == Token::opPlus) && i->type() != Token::LeftSquareBracket && isCommandParameterList(i)) { //Probably a command
				i = begin;
				delete var;
				return expectCommandCall(i);
			}
			delete var;
			i = begin;
		}
		case Token::Float:
		case Token::Integer:
		case Token::String:
		case Token::kFirst:
		case Token::kLast:
		case Token::kAfter:
		case Token::kBefore:
		case Token::kArraySize:
			return expectAssignementExpression(i);
		default:
			return 0;
	}
}

ast::Node *Parser::tryDelete(Parser::TokIterator &i) {
	if (i->type() == Token::kDelete) {
		CodePoint cp = i->codePoint();
		i++;

		ast::Node *p = expectExpressionList(i);

		ast::KeywordFunctionCall *call =  new ast::KeywordFunctionCall(ast::KeywordFunctionCall::Delete, cp);
		call->setParameters(p);
		return call;
	}
	return 0;
}

ast::Program *Parser::parse(const std::vector<Token> &tokens, Settings *settings) {
	mSettings = settings;

	std::vector<ast::TypeDefinition*> typeDefs;
	std::vector<ast::FunctionDefinition*> funcDefs;
	std::vector<ast::StructDefinition*> classDefs;
	ast::Block *block = new ast::Block(tokens.front().codePoint(), tokens.back().codePoint());
	TokIterator i = tokens.begin();
	while (i->type() != Token::EndOfTokens) {
		if (i->isEndOfStatement()) {
			i++;
			continue;
		}
		ast::TypeDefinition *type = tryTypeDefinition(i);
		if (mStatus == Error) return 0;
		if (type) {
			typeDefs.push_back(type);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}

		ast::StructDefinition *classDef = tryStructDefinition(i);
		if (mStatus == Error) return 0;
		if (classDef) {
			classDefs.push_back(classDef);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}

		ast::FunctionDefinition *func = tryFunctionDefinition(i);
		if (mStatus == Error) return 0;
		if (func) {
			funcDefs.push_back(func);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}

		ast::Node * n;
		for (int p = 0; p < blockParserCount; p++) {
			n =	(this->*(blockParsers[p]))(i);
			if (mStatus == Error) return 0;
			if (n) {
				expectEndOfStatement(i);
				if (mStatus == Error) return 0;
				break;
			}
		}
		if (n) {
			block->appendNode(n);
		}
		else {
			error(ErrorCodes::ecUnexpectedToken, "Unexpected token \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}
	}
	ast::Program *program = new ast::Program;
	program->setFunctionDefinitions(funcDefs);
	program->setTypeDefinitions(typeDefs);
	program->setStructDefinitions(classDefs);
	program->setMainBlock(block);

	return program;
}

ast::Node *Parser::tryConstDefinition(Parser::TokIterator &i) {
	if (i->type() == Token::kConst) {
		CodePoint cp = i->codePoint();
		i++;
		ast::Variable *var = expectVariable(i);
		if (mStatus == Error) return 0;
		if (i->type() != Token::opEqual) {
			error(ErrorCodes::ecExpectingAssignment, "Expecting '=' after the constant, got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}
		i++;
		ast::Node *value = expectExpression(i);
		if (mStatus == Error) return 0;
		ast::Const *ret = new ast::Const(cp);
		ret->setVariable(var);
		ret->setValue(value);
		return ret;
	}
	return 0;
}

ast::Node *Parser::tryGlobalDefinition(Parser::TokIterator &i) {
	if (i->type() == Token::kGlobal) {
		CodePoint cp = i->codePoint();
		i++;
		std::vector<ast::Node*> definitions;
		while (true) {
			ast::Node *def = expectVariableDefinitionOrArrayInitialization(i);
			if (mStatus == Error) return 0;

			definitions.push_back(def);

			if (i->type() != Token::Comma) break;
			i++;
		}

		ast::Global *global = new ast::Global(cp);
		global->setDefinitions(definitions);
		return global;
	}

	//Not global
	return 0;
}

ast::Node *Parser::tryVariableTypeDefinition(Parser::TokIterator &i) {
	ast::Node *r;
	if ((r = tryVariableTypeMark(i))) return r;
	if (i->type() == Token::kAs) {
		i++;
		return expectVariableTypeDefinition(i);
	}
	return 0;
}

ast::Node *Parser::tryVariableTypeMark(Parser::TokIterator &i) {
	switch (i->type()) {
		case Token::FloatTypeMark:
			return new ast::BasicType(ast::BasicType::Float, (i++)->codePoint());
		case Token::IntegerTypeMark:
			return new ast::BasicType(ast::BasicType::Integer, (i++)->codePoint());
		case Token::StringTypeMark:
			return new ast::BasicType(ast::BasicType::String, (i++)->codePoint());
		default:
			return 0;
	}
}

ast::Node *Parser::tryVariableAsType(Parser::TokIterator &i) {
	if (i->type() == Token::kAs) {
		i++;
		CodePoint cp = i->codePoint();

		ast::Identifier *id = expectIdentifierAfter(i, (i - 1)->toString());
		if (!id) return 0;
		ast::NamedType *namedType = new ast::NamedType(cp);
		namedType->setIdentifier(id);
		return namedType;
	}
	return 0;
}


ast::Node *Parser::tryReturn(Parser::TokIterator &i) {
	if (i->type() == Token::kReturn) {
		CodePoint cp = i->codePoint();
		i++;
		ast::Node *r = 0;
		if (!i->isEndOfStatement()) {
			r = expectExpression(i);
		}
		if (mStatus == Error) return 0;
		ast::Return *ret = new ast::Return(cp);
		ret->setValue(r);
		return ret;
	}
	return 0;
}

ast::TypeDefinition *Parser::tryTypeDefinition(Parser::TokIterator &i) {
	if (i->type() == Token::kType) {
		CodePoint startCp = i->codePoint();
		i++;
		ast::Identifier *id = expectIdentifier(i);
		if (mStatus == Error) return 0;
		expectEndOfStatement(i);
		if (mStatus == Error) return 0;

		std::vector<ast::Node *> fields;
		while (i->isEndOfStatement()) i++;
		while (i->type() == Token::kField) {
			CodePoint fCp = i->codePoint();
			i++;
			ast::Node *field = expectVariable(i); // FIXME? expectDefinitionOfVariableOrArray(i);
			if (mStatus == Error) return 0;
			fields.push_back(field);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;

			while (i->isEndOfStatement()) i++;
		}
		if (i->type() != Token::kEndType) {
			error(ErrorCodes::ecExpectingEndType, "Expecting \"EndType\", got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}


		ast::TypeDefinition *ret = new ast::TypeDefinition(startCp, i->codePoint());
		ret->setFields(fields);
		ret->setIdentifier(id);
		i++;
		return ret;
	}
	return 0;
}


ast::StructDefinition *Parser::tryStructDefinition(Parser::TokIterator &i) {
	if (i->type() == Token::kStruct) {
		CodePoint startCp = i->codePoint();
		i++;
		ast::Identifier *id = expectIdentifier(i);
		if (mStatus == Error) return 0;
		expectEndOfStatement(i);
		if (mStatus == Error) return 0;

		std::vector<ast::Node *> fields;
		while (i->isEndOfStatement()) i++;
		while (i->type() == Token::kField) {
			CodePoint fCp = i->codePoint();
			i++;
			ast::Node *field = expectVariable(i); // FIXME? expectDefinitionOfVariableOrArray(i);
			if (mStatus == Error) return 0;
			fields.push_back(field);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;

			while (i->isEndOfStatement()) i++;
		}
		if (i->type() != Token::kEndStruct) {
			error(ErrorCodes::ecExpectingEndType, "Expecting \"EndStruct\", got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}


		ast::StructDefinition *ret = new ast::StructDefinition(startCp, i->codePoint());
		ret->setFields(fields);
		ret->setIdentifier(id);
		i++;
		return ret;
	}
	return 0;
}

ast::Identifier *Parser::expectIdentifier(Parser::TokIterator &i) {
	if (i->type() == Token::Identifier) {
		ast::Identifier *id = new ast::Identifier( i->toString(), i->codePoint());
		i++;
		return id;
	}

	error(ErrorCodes::ecExpectingIdentifier, "Expecting an identifier, got \"" + i->toString() + "\"", i->codePoint());
	mStatus = Error;
	return 0;
}

ast::Identifier *Parser::expectIdentifierAfter(Parser::TokIterator &i, const std::string &after) {
	if (i->type() == Token::Identifier) {
		ast::Identifier *id = new ast::Identifier(i->toString(), i->codePoint());
		i++;
		return id;
	}

	error(ErrorCodes::ecExpectingIdentifier, (boost::format("Expecting an identifier after \"%2%\", got \"%1%\"") % i->toString() % after).str(), i->codePoint());
	mStatus = Error;
	return 0;
}

ast::ArrayInitialization *Parser::expectArrayInitialization(Parser::TokIterator &i) {
	CodePoint cp = i->codePoint();
	ast::Node *array = expectCallOrArraySubscriptExpression(i);
	if (mStatus == Error) return 0;
	ast::Node *varType = tryVariableAsType(i);
	if (mStatus == Error) {
		delete array;
		return 0;
	}
	if (!varType) varType = new ast::DefaultType(cp);

	if (array->type() != ast::Node::ntArraySubscript) {
		error(ErrorCodes::ecExpectingArraySubscript, "Expecting array subscript expression after Redim", i->codePoint());
		delete array;
		delete varType;
	}

	ast::ArraySubscript *subscript = array->cast<ast::ArraySubscript>();

	ast::ArrayInitialization *init = new ast::ArrayInitialization(cp);
	init->setArray(subscript->array());
	init->setDimensions(subscript->subscript());
	init->setValueType(varType);
	return init;
}

bool Parser::expectLeftParenthese(Parser::TokIterator &i) {
	if (i->type() != Token::LeftParenthese) {
		error(ErrorCodes::ecExpectingLeftParenthese, "Expecting a left parenthese after \"" + (i - 1)->toString() + "\"", i->codePoint());
		mStatus = Error;
		return false;
	}
	i++;
	return true;
}

bool Parser::expectRightParenthese(Parser::TokIterator &i) {
	if (i->type() != Token::RightParenthese) {
		error(ErrorCodes::ecExpectingRightParenthese, "Expecting a right parenthese after \"" + (i - 1)->toString() + "\"", i->codePoint());
		mStatus = Error;
		return false;
	}
	i++;
	return true;
}

bool Parser::expectLeftSquareBracket(Parser::TokIterator &i) {
	if (i->type() != Token::LeftSquareBracket) {
		error(ErrorCodes::ecExpectingLeftSquareBracket, "Expecting a left square bracket after \"" + (i - 1)->toString() + "\"", i->codePoint());
		mStatus = Error;
		return false;
	}
	i++;
	return true;
}

bool Parser::expectRightSquareBracket(Parser::TokIterator &i){
	if (i->type() != Token::RightSquareBracket) {
		error(ErrorCodes::ecExpectingRightSquareBracket, "Expecting a right square bracket after \"" + (i - 1)->toString() + "\"", i->codePoint());
		mStatus = Error;
		return false;
	}
	i++;
	return true;
}

ast::Node *Parser::expectVariableDefinitionOrArrayInitialization(Parser::TokIterator &i) {
	CodePoint cp = i->codePoint();
	ast::Identifier *id = expectIdentifierAfter(i, (i - 1)->toString());
	if (mStatus == Error) return 0;
	ast::Node *varType = tryVariableTypeDefinition(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::LeftSquareBracket) { //Sized array
		i++;
		ast::Node *dims = expectExpressionList(i);

		if (i->type() != Token::RightSquareBracket) {
			error(ErrorCodes::ecExpectingRightParenthese, "Expecting right parenthese, got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}
		i++;
		ast::Node *varType2 = tryVariableAsType(i);
		if (varType != 0) {
			if (varType2 != 0) {
				if (variableTypesAreEqual(varType, varType2)) {
					error(ErrorCodes::ecVariableTypeDefinedTwice, "Variable \"" + id->name() + "\" type defined twice", i->codePoint());
					mStatus = ErrorButContinue;
				}
				else {
					warning(WarningCodes::wcVariableTypeDefinedTwice, "Variable \"" + id->name() + "\" type defined twice", i->codePoint());
				}
			}
		}
		else {
			varType = varType2;
		}
		if (!varType) varType = new ast::DefaultType(cp);

		ast::ArrayInitialization *arr = new ast::ArrayInitialization(cp);
		arr->setArray(id);
		arr->setDimensions(dims);
		arr->setValueType(varType);
		return arr;
	}
	else {
		ast::Node *value = 0;
		if (!varType) varType = new ast::DefaultType(cp);
		if (i->type() == Token::opEqual) {
			i++;
			value = expectExpression(i);
			if (mStatus == Error) return 0;
		} else {
			value = new ast::DefaultValue(varType, cp);
		}


		ast::VariableDefinition *def = new ast::VariableDefinition(cp);
		def->setIdentifier(id);
		def->setValueType(varType);
		def->setValue(value);
		return def;
	}
}

ast::Node *Parser::expectPrimaryTypeDefinition(Parser::TokIterator &i) {
	switch (i->type()) {
		case Token::LeftParenthese: {
			i++;
			ast::Node *ty = expectVariableTypeDefinition(i);
			if (mStatus == Error) return 0;
			if (i->type() != Token::RightParenthese) {
				error(ErrorCodes::ecExpectingRightSquareBracket, "Expecting a right parenthese", i->codePoint());
				mStatus = Error;
				delete ty;
				return 0;
			}
			++i;
		}
		case Token::Identifier: {
			ast::Identifier *id = new ast::Identifier(i->toString(), i->codePoint());
			ast::NamedType *namedType = new ast::NamedType(i->codePoint());
			namedType->setIdentifier(id);
			++i;
			return namedType;
		}
		default:
			error(ErrorCodes::ecExpectingPrimaryExpression, "Expecting a primary type expression after \"" + (i - 1)->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
	}
}

ast::Node *Parser::expectArrayTypeDefinition(Parser::TokIterator &i) {
	ast::Node *base = expectPrimaryTypeDefinition(i);
	if (mStatus == Error) return 0;

	while(i->type() == Token::LeftSquareBracket) {
		CodePoint cp = i->codePoint();
		i++;
		int dims = 1;
		while (i->type() == Token::Comma) {
			++dims;
			i++;
		}
		if (i->type() != Token::RightSquareBracket) {
			error(ErrorCodes::ecExpectingRightSquareBracket, "Expecting a right square bracket", i->codePoint());
			mStatus = Error;
			delete base;
			return 0;
		}
		++i;

		ast::ArrayType *arrTy = new ast::ArrayType(cp);
		arrTy->setDimensions(dims);
		arrTy->setParentType(base);
		base = arrTy;
	}
	return base;
}

ast::Node *Parser::expectVariableTypeDefinition(Parser::TokIterator &i) {
	ast::Node *fpt = tryFunctionPointerTypeDefinition(i);
	if (mStatus == Error) return 0;
	if (fpt) return fpt;
	return expectArrayTypeDefinition(i);
}

ast::Node *Parser::tryFunctionPointerTypeDefinition(Parser::TokIterator &i) {
	if (i->type() != Token::kFunction) {
		return 0;
	}
	CodePoint cp = i->codePoint();
	i++;
	if (!expectLeftParenthese(i)) return 0;
	std::vector<ast::Node*> ns;
	while (true) {
		if (i->type() == Token::RightParenthese) {
			break;
		}
		ast::Node *t = expectVariableTypeDefinition(i);
		if (mStatus == Error) {
			containerDeleteAll(ns);
			return 0;
		}
		ns.push_back(t);
		if (i->type() == Token::Comma) {
			i++;
			continue;
		}
		break;
	}
	if (!expectRightParenthese(i)) return 0;
	ast::Node *retType = 0;
	if (i->type() == Token::kAs) {
		i++;
		retType = expectVariableTypeDefinition(i);
		if (mStatus == Error) {
			containerDeleteAll(ns);
			return 0;
		}
	}
	ast::List *paramTypeList = new ast::List(cp);
	paramTypeList->setItems(ns);
	ast::FunctionPointerType *n = new ast::FunctionPointerType(cp);
	n->setParameterTypes(paramTypeList);
	n->setReturnType(retType);
	return n;
}

void Parser::expectEndOfStatement(Parser::TokIterator &i) {
	if (i->isEndOfStatement()) {
		i++;
		return;
	}
	mStatus = Error;
	error(ErrorCodes::ecExpectingEndOfStatement, "Expecting end of line or ':', got \""+ i->toString() + "\"", i->codePoint());
	i++;
}

bool Parser::variableTypesAreEqual(ast::Node *a, ast::Node *b) {
	assert(a->type() == ast::Node::ntNamedType || a->type() == ast::Node::ntArrayType || a->type() == ast::Node::ntFunctionPointerType);
	assert(b->type() == ast::Node::ntNamedType || b->type() == ast::Node::ntArrayType || b->type() == ast::Node::ntFunctionPointerType);

	if (a->type() != b->type()) return false;
	if (a->type() == ast::Node::ntNamedType) {
		return static_cast<ast::NamedType*>(a)->identifier()->name() == static_cast<ast::NamedType*>(b)->identifier()->name();
	}
	else if (a->type() == ast::Node::ntArrayType){ // ast::Node::ntArrayType
		ast::ArrayType *arrTyA = static_cast<ast::ArrayType*>(a);
		ast::ArrayType *arrTyB = static_cast<ast::ArrayType*>(b);
		if (arrTyA->dimensions() != arrTyB->dimensions()) return false;
		return variableTypesAreEqual(arrTyA->parentType(), arrTyB->parentType());
	} else if (a->type() == ast::Node::ntFunctionPointerType) {
		ast::FunctionPointerType *funcPtrTyA = a->cast<ast::FunctionPointerType>();
		ast::FunctionPointerType *funcPtrTyB = b->cast<ast::FunctionPointerType>();
		if (funcPtrTyA->childNodeCount() != funcPtrTyB->childNodeCount()) return false;
		for (int i = 0; i < funcPtrTyA->childNodeCount(); i++) {
			if (!variableTypesAreEqual(funcPtrTyA->childNode(i), funcPtrTyB->childNode(i))) return false;
		}
		return true;
	}
	assert("WTF" && 0);
	return false;
}

bool Parser::isCommandParameterList(Parser::TokIterator i) {
	int level = 0;
	if (i->isEndOfStatement()) return true;
	while (!i->isEndOfStatement()) {
		switch ( i->type()) {
			case Token::LeftParenthese:
				level++; break;
			case Token::RightParenthese:
				level--; break;
			case Token::Comma:
				if (level == 0) {
					return true;
				}
				break;
			default:
				if (level == 0) {
					return true;
				}
		}
		++i;
	}
	return false;
}

void Parser::error(int code, std::string msg, CodePoint cp) {
	mErrorHandler->error(code, msg, cp);
}

void Parser::warning(int code, std::string msg, CodePoint cp) {
	mErrorHandler->warning(code, msg, cp);
}


ast::Variable *Parser::expectVariable(Parser::TokIterator &i) {
	ast::Variable* var = tryVariable(i);
	if (!var) {
		error(ErrorCodes::ecExpectingIdentifier, "Expecting a variable, got \"" + i->toString() + "\"", i->codePoint());
		mStatus = Error;
		return 0;
	}
	return var;
}


ast::Variable *Parser::tryVariable(Parser::TokIterator &i) {
	if (i->type() != Token::Identifier) {
		return 0;
	}
	CodePoint cp = i->codePoint();
	ast::Identifier *id = expectIdentifier(i);
	if (mStatus == Error) { return 0; }

	ast::Node *ty = tryVariableTypeDefinition(i);
	if (mStatus == Error) { delete id; return 0; }

	if (!ty) ty = new ast::DefaultType(cp);
	ast::Variable *var = new ast::Variable(cp);
	var->setIdentifier(id);
	var->setValueType(ty);
	return var;
}

ast::Node *Parser::expectVariableOrIdentifier(Parser::TokIterator &i) {
	if (i->type() != Token::Identifier) {
		error(ErrorCodes::ecExpectingIdentifier, "Expecting an identifier, got \"" + i->toString() + "\"", i->codePoint());
		mStatus = Error;
		return 0;
	}
	CodePoint cp = i->codePoint();
	ast::Identifier *id = expectIdentifier(i);
	if (mStatus == Error) { return 0; }

	ast::Node *ty = tryVariableTypeDefinition(i);
	if (mStatus == Error) { delete id; return 0; }

	if (!ty) return id;

	ast::Variable *var = new ast::Variable(cp);
	var->setIdentifier(id);
	var->setValueType(ty);
	return var;
}

ast::Node *Parser::trySelectStatement(Parser::TokIterator &i) {
	if (i->type() != Token::kSelect) return 0;
	//Select
	CodePoint startCp = i->codePoint();
	i++;
	ast::Node *var = expectExpression(i);
	if (mStatus == Error) return 0;
	expectEndOfStatement(i);
	if (mStatus == Error) return 0;
	std::vector<ast::SelectCase*> cases;
	ast::Node *defaultCase = 0;
	while (i->type() != Token::kEndSelect) {
		if (i->isEndOfStatement()) {
			i++;
			continue;
		}
		if (i->type() == Token::kCase) {
			CodePoint caseStartCp = i->codePoint();
			i++;
			ast::Node *val = expectExpression(i);
			if (mStatus == Error) return 0;
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			ast::Node *block = expectBlock(i);
			if (mStatus == Error) return 0;
			ast::SelectCase *c = new ast::SelectCase(caseStartCp, i->codePoint());
			c->setValue(val);
			c->setBlock(block);
			cases.push_back(c);
			continue;
		}
		else if (i->type() == Token::kDefault) {
			if (defaultCase != 0) {
				error(ErrorCodes::ecMultipleSelectDefaultCases, "Multiple default cases", i->codePoint());
				mStatus = ErrorButContinue;
			}
			i++;
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			defaultCase = expectBlock(i);
			if (mStatus == Error) return 0;
		}
		else {
			error(ErrorCodes::ecExpectingEndSelect, "Expecting \"EndSelect\", \"Case\" or \"Default\", got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}
	}
	ast::SelectStatement *ret = new ast::SelectStatement(startCp, i->codePoint());
	ret->setDefaultCase(defaultCase);
	ret->setCases(cases);
	ret->setVariable(var);
	i++;
	return ret;
}


ast::Node *Parser::tryGotoGosubAndLabel(Parser::TokIterator &i) {
	switch (i->type()) {
		case Token::kGoto: {
			ast::Goto *ret = new ast::Goto(i->codePoint());
			i++;
			ret->setLabel(expectIdentifier(i));
			if (mStatus == Error) { delete ret; return 0;}
			return ret;
		}
		case Token::kGosub: {
			ast::Gosub *ret = new ast::Gosub(i->codePoint());
			i++;
			ret->setLabel(expectIdentifier(i));
			if (mStatus == Error) { delete ret; return 0;}
			return ret;
		}
		case Token::Label: {
			ast::Label *label = new ast::Label(i->toString(), i->codePoint());
			i++;
			return label;
		}
		default:
			return 0;
	}
}

ast::Node *Parser::tryRedim(Parser::TokIterator &i) {
	if (i->type() != Token::kRedim) return 0;
	CodePoint cp = i->codePoint();
	i++;

	std::vector<ast::ArrayInitialization*> inits;
	do {
		ast::ArrayInitialization *arrInit = expectArrayInitialization(i);
		if (mStatus == Error) {
			containerDeleteAll(inits);
			return 0;
		}
		inits.push_back(arrInit);
	} while ((i++)->type() == Token::Comma);
	--i;

	ast::Redim *arr = new ast::Redim(cp);
	arr->setArrayInializations(inits);
	return arr;
}

ast::Node *Parser::tryDim(Parser::TokIterator &i) {
	if (i->type() == Token::kDim) {
		CodePoint cp = i->codePoint();
		i++;
		std::vector<ast::Node*> definitions;
		while (true) {
			ast::Node *def = expectVariableDefinitionOrArrayInitialization(i);
			if (mStatus == Error) return 0;

			definitions.push_back(def);

			if (i->type() != Token::Comma) break;
			i++;
		}

		ast::Dim *dim = new ast::Dim(cp);
		dim->setDefinitions(definitions);
		return dim;
	}
	return 0;
}

ast::Node *Parser::tryIfStatement(Parser::TokIterator &i) { //FINISH
	if (i->type() == Token::kIf) {
		CodePoint startCp = i->codePoint();
		i++;
		return expectIfStatementNoKeyword(startCp, i);
	}

	return 0;
}

ast::Node *Parser::expectIfStatementNoKeyword(const CodePoint &startCp, Parser::TokIterator &i) {
	ast::Node *condition = expectExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::kThen) {
		i++;
		if (!i->isEndOfStatement()) { //Inline if
			ast::Node *block = expectInlineBlock(i);
			if (mStatus == Error) return 0;
			if (i->type() == Token::kElseIf) {
				ast::Node *elseIf = expectElseIfStatement(i);
				if (mStatus == Error) return 0;
				ast::IfStatement *ret = new ast::IfStatement(startCp, i->codePoint());
				ret->setCondition(condition);
				ret->setBlock(block);
				ret->setElseBlock(elseIf);
				return ret;
			}
			if (i->type() == Token::kElse) {
				i++;
				ast::Node *elseBlock = expectInlineBlock(i);
				if (mStatus == Error) return 0;
				ast::IfStatement *ret = new ast::IfStatement(startCp, i->codePoint());
				ret->setCondition(condition);
				ret->setBlock(block);
				ret->setElseBlock(elseBlock);
				return ret;
			}

			ast::IfStatement *ret = new ast::IfStatement(startCp, i->codePoint());
			ret->setCondition(condition);
			ret->setBlock(block);
			return ret;
		}
		i++;
	}
	else {
		if (!i->isEndOfStatement()) {
			error(ErrorCodes::ecExpectingEndOfStatement, "Expecting the end of the line, ':', or \"then\", got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}
		i++;
	}

	ast::Node *block = expectBlock(i);
	ast::Node *elseBlock = 0;
	if (mStatus == Error) return 0;
	if (i->type() == Token::kElseIf) {
		elseBlock = expectElseIfStatement(i);
		if (mStatus == Error) return 0;
	}
	else if (i->type() == Token::kElse) {
		i++;
		elseBlock = expectBlock(i);
		if (mStatus == Error) return 0;
		if (i->type() != Token::kEndIf) {
			error(ErrorCodes::ecExpectingEndIf, "Expecting \"EndIf\" for if-statement, which begins at " + startCp.toString(), i->codePoint());
			mStatus = Error;
			return 0;
		}
		i++;
	}
	else {
		if (i->type() != Token::kEndIf) {
			error(ErrorCodes::ecExpectingEndIf, "Expecting \"EndIf\" for if-statement, which begins at " + startCp.toString(), i->codePoint());
			mStatus = Error;
			return 0;
		}
		i++;
	}


	ast::IfStatement *ret = new ast::IfStatement(startCp, (i - 1)->codePoint());
	ret->setElseBlock(elseBlock);
	ret->setCondition(condition);
	ret->setBlock(block);

	return ret;
}

ast::Node *Parser::expectElseIfStatement(Parser::TokIterator &i) {
	assert(i->type() == Token::kElseIf);
	CodePoint startCp = i->codePoint();
	i++;
	return expectIfStatementNoKeyword(startCp, i);
}

ast::Node *Parser::tryWhileStatement(Parser::TokIterator &i) {
	if (i->type() == Token::kWhile) {
		CodePoint startCp = i->codePoint();
		i++;

		ast::Node *cond = expectExpression(i);
		if (mStatus == Error) return 0;

		expectEndOfStatement(i);
		if (mStatus == Error) return 0;

		ast::Block *block = expectBlock(i)->cast<ast::Block>();
		if (mStatus == Error) return 0;

		if (i->type() != Token::kWend) {
			error(ErrorCodes::ecExpectingWend, (boost::format("Expecting \"Wend\" to end \"While\" on line %1%, got %2%") % boost::lexical_cast<std::string>(startCp.line()) % i->toString()).str(),i->codePoint());
			mStatus = Error;
			return 0;
		}
		ast::WhileStatement *ret = new ast::WhileStatement(startCp, i->codePoint());
		ret->setBlock(block);
		ret->setCondition(cond);
		i++;
		return ret;

	}
	return 0;
}


ast::FunctionDefinition *Parser::tryFunctionDefinition(Parser::TokIterator &i) {
	if (i->type() == Token::kFunction) {
		CodePoint startCp = i->codePoint();
		i++;
		ast::Identifier *functionId = expectIdentifier(i);
		if (mStatus == Error) return 0;

		ast::Node *retType = tryVariableTypeDefinition(i);
		if (mStatus == Error) return 0;

		if (i->type() != Token::LeftParenthese) {
			error(ErrorCodes::ecExpectingLeftParenthese, "Expecting '(' after a function name, got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}
		i++;

		ast::Node* args = 0;
		if (i->type() != Token::RightParenthese) {
			args = expectFunctionParameterList(i);
			if (mStatus == Error) return 0;
			if (i->type() != Token::RightParenthese) {
				error(ErrorCodes::ecExpectingRightParenthese, "Expecting ')', got \"" + i->toString() + "\"", i->codePoint());
				mStatus = Error;
				return 0;
			}
		}
		else {
			args = new ast::List(i->codePoint());
		}
		i++;
		ast::Node *retType2 = tryVariableAsType(i);
		if (mStatus == Error) return 0;
		if (retType != 0 && retType2 != 0) {
			if (variableTypesAreEqual(retType, retType2)) {
				warning(WarningCodes::wcFunctionReturnTypeDefinedTwice, "Function return type defined twice", i->codePoint());
			}
			else {
				error(ErrorCodes::ecFunctionReturnTypeDefinedTwice, "Function return type defined twice", i->codePoint());
				mStatus = ErrorButContinue;
			}
		}
		if (retType2) retType = retType2;

		expectEndOfStatement(i);
		ast::Node *block = expectBlock(i);
		if (i->type() != Token::kEndFunction) {
			error(ErrorCodes::ecExpectingEndFunction, "Expecting \"EndFunction\", got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
		}
		ast::FunctionDefinition *func = new ast::FunctionDefinition(startCp, i->codePoint());
		func->setBlock(block);
		func->setIdentifier(functionId);
		func->setParameterList(args);
		func->setReturnType(retType);
		i++;

		return func;
	}
	return 0;
}

static ast::ExpressionNode::Op tokenTypeToOperator(Token::Type t) {
	switch (t) {
		case Token::opEqual:
			return ast::ExpressionNode::opEqual;
		case Token::opNotEqual:
			return ast::ExpressionNode::opNotEqual;
		case Token::opGreater:
			return ast::ExpressionNode::opGreater;
		case Token::opLess:
			return ast::ExpressionNode::opLess;
		case Token::opGreaterEqual:
			return ast::ExpressionNode::opGreaterEqual;
		case Token::opLessEqual:
			return ast::ExpressionNode::opLessEqual;
		case Token::opPlus:
			return ast::ExpressionNode::opAdd;
		case Token::opMinus:
			return ast::ExpressionNode::opSubtract;
		case Token::opMultiply:
			return ast::ExpressionNode::opMultiply;
		case Token::opPower:
			return ast::ExpressionNode::opPower;
		case Token::opMod:
			return ast::ExpressionNode::opMod;
		case Token::opShl:
			return ast::ExpressionNode::opShl;
		case Token::opShr:
			return ast::ExpressionNode::opShr;
		case Token::opSar:
			return ast::ExpressionNode::opSar;
		case Token::opDivide:
			return ast::ExpressionNode::opDivide;
		case Token::opAnd:
			return ast::ExpressionNode::opAnd;
		case Token::opOr:
			return ast::ExpressionNode::opOr;
		case Token::opXor:
			return ast::ExpressionNode::opXor;
		default:
			return ast::ExpressionNode::opInvalid;
	}
}

ast::Node *Parser::expectAssignementExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectCallOrArraySubscriptExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opEqual) {
		CodePoint cp2 = i->codePoint();
		++i;
		ast::Node *second = expectLogicalOrExpression(i);
		if (mStatus == Error) { delete first; return 0; }
		ast::Expression *expr = new ast::Expression(ast::Expression::RightToLeft, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(ast::ExpressionNode::opAssign, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);
		while (i->type() == Token::opEqual) {
			exprNode = new ast::ExpressionNode(ast::ExpressionNode::opAssign, i->codePoint());
			expr->appendOperation(exprNode);
			i++;

			ast::Node *operand = expectLogicalOrExpression(i);
			if (mStatus == Error) { delete expr; return 0; }

			exprNode->setOperand(operand);
		}
		return expr;
	}

	return first;
}

ast::Node *Parser::expectExpression(Parser::TokIterator &i) {
	return expectLogicalOrExpression(i);

}

ast::Node *Parser::expectLogicalOrExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectLogicalAndExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opOr || i->type() == Token::opXor) {
		CodePoint cp2 = i->codePoint();
		ast::ExpressionNode::Op op = tokenTypeToOperator(i->type());
		i++;
		ast::Node *second = expectLogicalAndExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression(ast::Expression::LeftToRight, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(op, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);

		while (i->type() == Token::opOr || i->type() == Token::opXor) {
			ast::ExpressionNode::Op op = tokenTypeToOperator(i->type());
			exprNode = new ast::ExpressionNode(op, i->codePoint());
			expr->appendOperation(exprNode);
			i++;
			ast::Node *ep = expectLogicalAndExpression(i);
			if (mStatus == Error) return 0;
			exprNode->setOperand(ep);
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectLogicalAndExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectEqualityExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opAnd) {
		CodePoint cp2 = i->codePoint();
		i++;
		ast::Node *second = expectEqualityExpression(i);
		if (mStatus == Error) return 0;

		ast::Expression *expr = new ast::Expression(ast::Expression::LeftToRight, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(ast::ExpressionNode::opAnd, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);
		while (i->type() == Token::opAnd) {
			exprNode = new ast::ExpressionNode(ast::ExpressionNode::opAnd, i->codePoint());
			expr->appendOperation(exprNode);
			i++;
			ast::Node *ep = expectEqualityExpression(i);
			if (mStatus == Error) { delete expr; return 0; }
			exprNode->setOperand(ep);
		}
		return expr;
	}
	else {
		return first;
	}
}


ast::Node *Parser::expectEqualityExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectRelativeExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opEqual || i->type() == Token::opNotEqual) {
		CodePoint cp2 = i->codePoint();
		ast::ExpressionNode::Op op = tokenTypeToOperator(i->type());
		i++;
		ast::Node *second = expectRelativeExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression(ast::Expression::LeftToRight, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(op, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);
		while (i->type() == Token::opEqual || i->type() == Token::opNotEqual) {
			op = tokenTypeToOperator(i->type());
			exprNode = new ast::ExpressionNode(op, i->codePoint());
			expr->appendOperation(exprNode);
			i++;
			ast::Node *ep = expectRelativeExpression(i);
			if (mStatus == Error) { delete expr; return 0; }
			exprNode->setOperand(ep);
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectRelativeExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectAdditiveExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opGreater || i->type() == Token::opLess || i->type() == Token::opGreaterEqual || i->type() == Token::opLessEqual) {
		CodePoint cp2 = i->codePoint();
		ast::ExpressionNode::Op op = tokenTypeToOperator(i->type());
		i++;
		ast::Node *second = expectAdditiveExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression(ast::Expression::LeftToRight, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(op, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);
		while (i->type() == Token::opGreater || i->type() == Token::opLess || i->type() == Token::opGreaterEqual || i->type() == Token::opLessEqual) {
			op = tokenTypeToOperator(i->type());
			exprNode = new ast::ExpressionNode(op, i->codePoint());
			expr->appendOperation(exprNode);
			i++;
			ast::Node *ep = expectAdditiveExpression(i);
			if (mStatus == Error) { delete expr; return 0; }
			exprNode->setOperand(ep);
		}
		return expr;
	}
	else {
		return first;
	}
}


ast::Node *Parser::expectAdditiveExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectMultiplicativeExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opPlus || i->type() == Token::opMinus) {
		CodePoint cp2 = i->codePoint();
		ast::ExpressionNode::Op op = tokenTypeToOperator(i->type());
		i++;
		ast::Node *second = expectMultiplicativeExpression(i);
		if (mStatus == Error) return 0;

		ast::Expression *expr = new ast::Expression(ast::Expression::LeftToRight, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(op, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);

		while (i->type() == Token::opPlus || i->type() == Token::opMinus) {
			op = tokenTypeToOperator(i->type());
			exprNode = new ast::ExpressionNode(op, i->codePoint());
			expr->appendOperation(exprNode);
			i++;
			ast::Node *ep = expectMultiplicativeExpression(i);
			if (mStatus == Error) { delete expr; return 0; }
			exprNode->setOperand(ep);
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectMultiplicativeExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectBitShiftExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opMultiply || i->type() == Token::opDivide || i->type() == Token::opMod) {
		CodePoint cp2 = i->codePoint();
		ast::ExpressionNode::Op op = tokenTypeToOperator(i->type());
		i++;
		ast::Node *second = expectBitShiftExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression(ast::Expression::LeftToRight, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(op, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);
		while (i->type() == Token::opMultiply || i->type() == Token::opDivide || i->type() == Token::opMod) {
			op = tokenTypeToOperator(i->type());
			exprNode = new ast::ExpressionNode(op, i->codePoint());
			expr->appendOperation(exprNode);
			i++;
			ast::Node *ep = expectBitShiftExpression(i);
			if (mStatus == Error) { delete expr; return 0; }
			exprNode->setOperand(ep);
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectBitShiftExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectPowExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opShl || i->type() == Token::opShr || i->type() == Token::opSar) {
		CodePoint cp2 = i->codePoint();
		ast::ExpressionNode::Op op = tokenTypeToOperator(i->type());
		i++;
		ast::Node *second = expectPowExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression(ast::Expression::LeftToRight, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(op, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);
		while (i->type() == Token::opShl || i->type() == Token::opShr || i->type() == Token::opSar) {
			op = tokenTypeToOperator(i->type());
			exprNode = new ast::ExpressionNode(op, i->codePoint());
			expr->appendOperation(exprNode);
			i++;
			ast::Node *ep = expectPowExpression(i);
			if (mStatus == Error) { delete expr; return 0; }
			exprNode->setOperand(ep);
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectPowExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectUnaryExpession(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opPower) {
		CodePoint cp2 = i->codePoint();
		i++;
		ast::Node *second = expectUnaryExpession(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression(ast::Expression::LeftToRight, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(ast::ExpressionNode::opPower, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);
		while (i->type() == Token::opPower) {
			exprNode = new ast::ExpressionNode(ast::ExpressionNode::opPower, i->codePoint());
			expr->appendOperation(exprNode);
			i++;
			ast::Node *ep = expectUnaryExpession(i);
			if (mStatus == Error) { delete expr; return 0; }
			exprNode->setOperand(ep);
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectUnaryExpession(Parser::TokIterator &i) {
	CodePoint cp = i->codePoint();
	ast::Unary::Op op;
	switch (i->type()) {
		case Token::opPlus:
			op = ast::Unary::opPositive;
			break;
		case Token::opMinus: {
			op = ast::Unary::opNegative;
			++i;
			ast::Node *n = tryNegativeLiteral(i);
			if (n) return n;
			--i;
			break;
		}
		case Token::opNot:
			op = ast::Unary::opNot;
			break;
		default:
			return expectCallOrArraySubscriptExpression(i);
	}
	i++;
	ast::Node *expr = expectCallOrArraySubscriptExpression(i);
	if (mStatus == Error) return 0;
	ast::Unary *unary = new ast::Unary(op, cp);
	unary->setOperand(expr);
	return unary;
}

ast::Node *Parser::expectCallOrArraySubscriptExpression(Parser::TokIterator &i) {
	ast::Node *base = expectPrimaryExpression(i);
	if (mStatus == Error) return 0;
	while (true) {
		switch (i->type()) {
			case Token::LeftParenthese: {
				CodePoint cp = i->codePoint();
				i++;
				ast::Node *params = 0;
				if (i->type() != Token::RightParenthese) {
					params = expectExpressionList(i);
					if (mStatus == Error) { return 0; }
				}
				else {
					params = new ast::List(i->codePoint());
				}

				if (i->type() != Token::RightParenthese) {
					error(ErrorCodes::ecExpectingRightParenthese, "Expecting a right parenthese, got \"" + i->toString() + "\"", i->codePoint());
					mStatus = Error;
					delete params;
					return 0;
				}
				++i;

				ast::FunctionCall *call = new ast::FunctionCall(cp);
				call->setFunction(base);
				call->setParameters(params);
				base = call;
				break;
			}
			case Token::LeftSquareBracket: {
				CodePoint cp = i->codePoint();
				i++;
				ast::Node *params = 0;
				if (i->type() != Token::RightSquareBracket) {
					params = expectExpressionList(i);
					if (mStatus == Error) { return 0; }
				}
				else {
					params = new ast::List(i->codePoint());
				}

				if (i->type() != Token::RightSquareBracket) {
					error(ErrorCodes::ecExpectingRightSquareBracket, "Expecting a right square bracket, got \"" + i->toString() + "\"", i->codePoint());
					mStatus = Error;
					delete params;
					return 0;
				}
				++i;

				ast::ArraySubscript *s = new ast::ArraySubscript(cp);
				s->setArray(base);
				s->setSubscript(params);
				base = s;
				break;
			}
			case Token::opDot: {
				CodePoint cp = i->codePoint();
				i++;
				ast::Expression *expr = 0;
				if (base->type() == ast::Node::ntExpression) {
					expr = base->cast<ast::Expression>();
					if (expr->associativity() != ast::Expression::LeftToRight) {
						expr = 0;
					}
				}
				if (!expr) {
					expr = new ast::Expression(ast::Expression::LeftToRight, cp);
					expr->setFirstOperand(base);
				}

				ast::Node *identifier = expectVariableOrIdentifier(i);
				ast::ExpressionNode *exprNode = new ast::ExpressionNode(ast::ExpressionNode::opMember, cp);

				exprNode->setOperand(identifier);
				expr->appendOperation(exprNode);
				base = expr;
				break;
			}
			default:
				return base;
		}
	}
}

ast::Node *Parser::tryNegativeLiteral(Parser::TokIterator &i) {
	switch (i->type()) {
		case Token::Integer: {
			int val = 0;
			try {
				val = -boost::lexical_cast<int>(i->toString());
			} catch (boost::bad_lexical_cast &e) {
				error(ErrorCodes::ecCantParseInteger, "Cannot parse an integer \"-" + i->toString() + "\"", i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer(val, i->codePoint());
			i++;
			return intN;
		}
		case Token::IntegerHex: {
			int val = 0;
			try {
				val = -boost::lexical_cast<int>("0x" + i->toString());
			} catch (boost::bad_lexical_cast &e) {
				error(ErrorCodes::ecCantParseInteger, "Cannot parse an integer \"-" + i->toString() + "\"", i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer(val, i->codePoint());
			i++;
			return intN;
		}

		case Token::Float: {
			float val;
			try {
				if (i->toString().front() == '.') { //leading dot .13123
					val = -boost::lexical_cast<float>("0" + i->toString());
				}
				else if (i->toString().back() == '.') { //Ending dot 1231.
					val = -boost::lexical_cast<float>(i->toString() + '0');
				}
				else {
					val = -boost::lexical_cast<float>(i->toString());
				}
			}
			catch (boost::bad_lexical_cast &e) {
				error(ErrorCodes::ecCantParseFloat, "Cannot parse float \"-" + i->toString() + "\"", i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Float *f = new ast::Float(val, i->codePoint());
			i++;
			return f;
		}
		default:
			return 0;
	}
}



ast::Node *Parser::expectPrimaryExpression(TokIterator &i) {
	switch (i->type()) {
		case Token::LeftParenthese: {
			i++;
			ast::Node *expr = expectExpression(i);
			if (i->type() != Token::RightParenthese) {
				error(ErrorCodes::ecExpectingRightParenthese, "Expecting a right parenthese, got \"" + i->toString() + "\"", i->codePoint());
				mStatus = Error;
				return 0;
			}
			i++;
			return expr;
		}
		case Token::Integer: {
			int val = 0;
			try {
				val = boost::lexical_cast<int>(i->toString());
			} catch (boost::bad_lexical_cast &e) {
				error(ErrorCodes::ecCantParseInteger, "Cannot parse an integer \"" + i->toString() + "\"", i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer(val, i->codePoint());
			i++;
			return intN;
		}
		case Token::IntegerHex: {
			int val = 0;
			try {
				val = boost::lexical_cast<int>("0x" + i->toString());
			} catch (boost::bad_lexical_cast &e) {
				error(ErrorCodes::ecCantParseInteger, "Cannot parse an integer \"" + i->toString() + "\"", i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer(val, i->codePoint());
			i++;
			return intN;
		}

		case Token::Float: {
			float val;
			try {
				if (i->toString().front() == '.') { //leading dot .13123
					val = boost::lexical_cast<float>("0" + i->toString());
				}
				else if (i->toString().back() == '.') { //Ending dot 1231.
					val = boost::lexical_cast<float>(i->toString() + '0');
				}
				else {
					val = boost::lexical_cast<float>(i->toString());
				}
			}
			catch (boost::bad_lexical_cast &e) {
				error(ErrorCodes::ecCantParseFloat, "Cannot parse float \"" + i->toString() + "\"", i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Float *f = new ast::Float(val, i->codePoint());
			i++;
			return f;
		}
		case Token::String: {
			ast::String *str = new ast::String(i->toString(), i->codePoint());
			i++;
			return str;
		}
		case Token::Identifier: {
			CodePoint cp = i->codePoint();
			ast::Identifier *identifier = new ast::Identifier(i->toString(), i->codePoint());
			i++;
			ast::Node *varType = tryVariableTypeDefinition(i);
			if (varType) {
				ast::Variable *var = new ast::Variable(cp);
				var->setIdentifier(identifier);
				var->setValueType(varType);
				return var;
			}
			else {
				return identifier;
			}
		}
		case Token::kNew:
		case Token::kFirst:
		case Token::kLast:
		case Token::kBefore:
		case Token::kAfter:
		case Token::kArraySize: {
			ast::KeywordFunctionCall *ret;
			switch (i->type()) {
				case Token::kNew:
					ret = new ast::KeywordFunctionCall(ast::KeywordFunctionCall::New, i->codePoint()); break;
				case Token::kFirst:
					ret = new ast::KeywordFunctionCall(ast::KeywordFunctionCall::First, i->codePoint()); break;
				case Token::kLast:
					ret = new ast::KeywordFunctionCall(ast::KeywordFunctionCall::Last, i->codePoint()); break;
				case Token::kBefore:
					ret = new ast::KeywordFunctionCall(ast::KeywordFunctionCall::Before, i->codePoint()); break;
				case Token::kAfter:
					ret = new ast::KeywordFunctionCall(ast::KeywordFunctionCall::After, i->codePoint()); break;
				case Token::kArraySize:
					ret = new ast::KeywordFunctionCall(ast::KeywordFunctionCall::ArraySize, i->codePoint()); break;
				default:
					assert("WTF assertion");
					ret = 0;
			}
			i++;

			if (!expectLeftParenthese(i)) {
				delete ret;
				return 0;
			}
			ret->setParameters(expectExpressionList(i));
			if (mStatus == Error) {
				delete ret;
				return 0;
			}
			if (!expectRightParenthese(i)) {
				delete ret;
				return 0;
			}

			return ret;
		}
		default:
			error(ErrorCodes::ecExpectingPrimaryExpression, "Expecting a primary expression, got \"" + i->toString() + "\"", i->codePoint());
			mStatus = Error;
			return 0;
	}
}


ast::Node *Parser::expectExpressionList(Parser::TokIterator &i) {
	ast::List *list = new ast::List(i->codePoint());
	--i;
	do {
		++i;
		ast::Node *expr = expectExpression(i);
		if (mStatus == Error) {
			delete list;
			return 0;
		}
		list->appendItem(expr);
	} while (i->type() == Token::Comma);

	if (list->childNodeCount() == 1) {
		ast::Node *onlyItem = list->childNode(0);
		list->takeAll();
		delete list;
		return onlyItem;
	}
	return list;
}

ast::Node *Parser::expectCommandCall(Parser::TokIterator &i) {
	CodePoint cp = i->codePoint();
	ast::Identifier *id = expectIdentifier(i);
	if (mStatus == Error) return 0;

	ast::Node *params;
	if (i->isEndOfStatement()) {
		params = new ast::List(i->codePoint());
	}
	else {
		params = expectExpressionList(i);
	}

	if (mStatus == Error) return 0;
	ast::FunctionCall *call = new ast::FunctionCall(cp);
	call->setFunction(id);
	call->setParameters(params);
	call->setIsCommand(true);
	return call;
}

ast::Node *Parser::expectVariableDefinitionList(Parser::TokIterator &i) {
	CodePoint cp = i->codePoint();
	i++;
	std::vector<ast::Node*> definitions;
	while (true) {
		ast::Node *def = expectVariableDefinitionOrArrayInitialization(i);
		if (mStatus == Error) return 0;

		definitions.push_back(def);

		if (i->type() != Token::Comma) break;
		i++;
	}
	if (definitions.size() == 1) {
		return definitions.front();
	}
	ast::List *ret = new ast::List(cp);
	ret->setItems(definitions);
	return ret;
}

ast::Node *Parser::expectFunctionParameterList(Parser::TokIterator &i) {
	if (i->type() == Token::RightParenthese) {
		return new ast::List(i->codePoint());
	}

	ast::List *list = new ast::List(i->codePoint());
	--i;
	do {
		++i;
		ast::Node *param = expectVariableDefinition(i);
		list->appendItem(param);
		if (mStatus == Error) {
			delete list;
			return 0;
		}
	} while (i->type() == Token::Comma);
	return list;
}

ast::Node *Parser::expectVariableDefinition(Parser::TokIterator &i) {
	CodePoint cp = i->codePoint();
	ast::Identifier *id = expectIdentifierAfter(i, (i - 1)->toString());
	if (mStatus == Error) return 0;

	ast::Node *varType = tryVariableTypeDefinition(i);
	if (mStatus == Error) return 0;
	if (!varType) varType = new ast::DefaultType(id->codePoint());
	ast::Node *value = 0;
	if (i->type() == Token::opEqual) {
		i++;
		value = expectExpression(i);
		if (mStatus == Error) return 0;
	} else {
		value = new ast::DefaultValue(varType, cp);
	}
	ast::VariableDefinition *def = new ast::VariableDefinition(cp);
	def->setIdentifier(id);
	def->setValueType(varType);
	def->setValue(value);
	return def;
}


ast::Node *Parser::tryRepeatStatement(Parser::TokIterator &i) {
	if (i->type() == Token::kRepeat) {
		CodePoint startCp = i->codePoint();
		i++;
		expectEndOfStatement(i);
		if (mStatus == Error) return 0;
		ast::Node *block = expectBlock(i);
		if (mStatus == Error) return 0;
		if (i->type() == Token::kUntil) {
			CodePoint endCp = i->codePoint();
			i++;
			ast::Node *condition = expectExpression(i);
			if (mStatus == Error) return 0;
			ast::RepeatUntilStatement *ret = new ast::RepeatUntilStatement(startCp, endCp);
			ret->setCondition(condition);
			ret->setBlock(block);
			return ret;
		}
		if (i->type() == Token::kForever) {
			ast::RepeatForeverStatement *ret = new ast::RepeatForeverStatement(startCp, i->codePoint());
			ret->setBlock(block);
			i++;
			return ret;
		}
		error(ErrorCodes::ecExpectingEndOfRepeat, "Expecting \"Until\" or \"Forever\", got \"" + i->toString() + "\"", i->codePoint());
		mStatus = Error;
		return 0;
	}

	return 0;
}

ast::Node *Parser::tryForStatement(Parser::TokIterator &i) {
	if (i->type() == Token::kFor) {
		CodePoint startCp = i->codePoint();
		i++;

		Parser::TokIterator start = i;
		ast::Node *part1 = expectVariable(i);
		if (mStatus == Error) { return 0;}

		if (i->type() == Token::opEqual) {
			i++;
			if (i->type() == Token::kEach) { //For-Each
				i++;

				ast::Node *container = expectVariable(i);
				if (mStatus == Error) {delete part1; return 0;}

				expectEndOfStatement(i);
				if (mStatus == Error) {delete part1; delete container; return 0;}

				ast::Node *block = expectBlock(i);
				if (mStatus == Error) {delete part1; delete container; return 0;}

				if (i->type() != Token::kNext) {
					error(ErrorCodes::ecExpectingNext, "Expecting \"Next\" to end For-Each block starting at line " + boost::lexical_cast<int>(startCp.line()), i->codePoint());
					mStatus = Error;
					delete part1;
					delete container;
					delete block;
					return 0;
				}


				ast::ForEachStatement *forEach = new ast::ForEachStatement(startCp, i->codePoint());
				forEach->setVariable(part1);
				forEach->setContainer(container);
				forEach->setBlock(block);
				i++;

				CodePoint nextVCp = i->codePoint();
				ast::Node *nextV = tryVariable(i);
				if (mStatus == Error) {
					delete forEach;
					return 0;
				}
				/*if (nextV) {
					warning(WarningCodes::wcNextVariableIgnored, "The variable name after \"Next\" is ignored", nextVCp);
					delete nextV;
				}*/
				return forEach;
			}

			//For-To
			i = start;
			delete part1;
			part1 = expectAssignementExpression(i);
			if (mStatus == Error) { return 0;}
		}

		if (i->type() != Token::kTo) {
			error(ErrorCodes::ecExpectingTo, "Expecting \"To\" after \"" + (i - 1)->toString() + "\"", i->codePoint());
			mStatus = Error;
			delete part1;
			return 0;
		}
		i++;

		ast::Node *to = expectExpression(i);
		ast::Node *step = 0;
		if (i->type() == Token::kStep) {
			i++;
			step = expectExpression(i);
		}

		expectEndOfStatement(i);
		if (mStatus == Error) { delete part1; delete to; if (step) delete step;  return 0;}

		ast::Node *block = expectBlock(i);
		if (mStatus == Error) { delete part1; delete to; if (step) delete step;  return 0;}

		if (i->type() != Token::kNext) {
			error(ErrorCodes::ecExpectingNext, "Expecting \"Next\" to end For-Each block starting at line " + boost::lexical_cast<int>(startCp.line()), i->codePoint());
			mStatus = Error;
			delete part1;
			delete to;
			if (step) delete step;
			delete block;
			return 0;
		}
		ast::ForToStatement *forTo = new ast::ForToStatement(startCp, i->codePoint());
		forTo->setFrom(part1);
		forTo->setTo(to);
		forTo->setStep(step);
		forTo->setBlock(block);
		i++;


		CodePoint nextVCp = i->codePoint();
		ast::Node *nextV = tryVariable(i);
		if (mStatus == Error) {
			delete forTo;
			return 0;
		}
		if (nextV) {
			//warning(WarningCodes::wcNextVariableIgnored, tr("The variable name after \"Next\" is ignored"), nextVCp);
			delete nextV;
		}
		return forTo;
	}
	return 0;
}

ast::Node *Parser::tryExit(Parser::TokIterator &i) {
	if (i->type() == Token::kExit) {
		return new ast::Exit((i++)->codePoint());
	}
	return 0;
}



