#include "parser.h"
#include "errorcodes.h"
#include "warningcodes.h"
#include <assert.h>
Parser::Parser():
	mStatus(Ok) {
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
	&Parser::tryExpression
};
static const int blockParserCount = 13;

ast::Node *Parser::expectBlock(Parser::TokIterator &i) {
	QList<ast::Node*> statements;
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
			statements.append(n);
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
	QList<ast::Node*> nodes;
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
					nodes.append(n);
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
			nodes.append(n);
		}
		else {
			break;
		}
	}
	ast::Block *block = new ast::Block(startCp, i->codePoint());
	block->setChildNodes(nodes);
	return block;
}

ast::Node *Parser::tryExpression(Parser::TokIterator &i) {
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
			return expectExpression(i);
		default:
			return 0;
	}
}

ast::Program *Parser::parse(const QList<Token> &tokens, const Settings &settings) {
	mSettings = settings;

	QList<ast::TypeDefinition*> typeDefs;
	QList<ast::FunctionDefinition*> funcDefs;
	QList<ast::StructDefinition*> classDefs;
	ast::Block *block = new ast::Block(tokens.first().codePoint(), tokens.last().codePoint());
	TokIterator i = tokens.begin();
	while (i->type() != Token::EndOfTokens) {
		if (i->isEndOfStatement()) {
			i++;
			continue;
		}
		ast::TypeDefinition *type = tryTypeDefinition(i);
		if (mStatus == Error) return 0;
		if (type) {
			typeDefs.append(type);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}

		ast::StructDefinition *classDef = tryStructDefinition(i);
		if (mStatus == Error) return 0;
		if (classDef) {
			classDefs.append(classDef);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}

		ast::FunctionDefinition *func = tryFunctionDefinition(i);
		if (mStatus == Error) return 0;
		if (func) {
			funcDefs.append(func);
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
			emit error(ErrorCodes::ecUnexpectedToken, tr("Unexpected token \"%1\"").arg(i->toString()), i->codePoint());
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
		if (i->type() != Token::opAssign) {
			emit error(ErrorCodes::ecExpectingAssignment, tr("Expecting '=' after the constant, got \"%1\"").arg(i->toString()), i->codePoint());
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
		QList<ast::Node*> definitions;
		while (true) {
			ast::Node *def = expectVariableDefinitionOrArrayInitialization(i);
			if (mStatus == Error) return 0;

			definitions.append(def);

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

		QList<ast::Node *> fields;
		while (i->isEndOfStatement()) i++;
		while (i->type() == Token::kField) {
			CodePoint fCp = i->codePoint();
			i++;
			ast::Node *field = expectVariable(i); // FIXME? expectDefinitionOfVariableOrArray(i);
			if (mStatus == Error) return 0;
			fields.append(field);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;

			while (i->isEndOfStatement()) i++;
		}
		if (i->type() != Token::kEndType) {
			emit error(ErrorCodes::ecExpectingEndType, tr("Expecting \"EndType\", got \"%1\"").arg(i->toString()), i->codePoint());
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

		QList<ast::Node *> fields;
		while (i->isEndOfStatement()) i++;
		while (i->type() == Token::kField) {
			CodePoint fCp = i->codePoint();
			i++;
			ast::Node *field = expectVariable(i); // FIXME? expectDefinitionOfVariableOrArray(i);
			if (mStatus == Error) return 0;
			fields.append(field);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;

			while (i->isEndOfStatement()) i++;
		}
		if (i->type() != Token::kEndStruct) {
			emit error(ErrorCodes::ecExpectingEndType, tr("Expecting \"EndStruct\", got \"%1\"").arg(i->toString()), i->codePoint());
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

	emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting an identifier, got \"%1\"").arg(i->toString()), i->codePoint());
	mStatus = Error;
	return 0;
}

ast::Identifier *Parser::expectIdentifierAfter(Parser::TokIterator &i, const QString &after) {
	if (i->type() == Token::Identifier) {
		ast::Identifier *id = new ast::Identifier(i->toString(), i->codePoint());
		i++;
		return id;
	}

	emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting an identifier after \"%2\", got \"%1\"").arg(i->toString(), after), i->codePoint());
	mStatus = Error;
	return 0;
}

ast::ArrayInitialization *Parser::expectArrayInitialization(Parser::TokIterator &i) {
	CodePoint cp = i->codePoint();
	ast::Identifier *id = expectIdentifier(i);
	if (mStatus == Error) return 0;
	ast::Node *varType = tryVariableTypeDefinition(i);
	if (mStatus == Error) { delete id; return 0; }
	if (!expectLeftSquareBracket(i)) { delete id; return 0; }
	ast::Node *dims = expectExpressionList(i);
	if (mStatus == Error) { delete id; return 0; }
	if (!expectRightSquareBracket(i)) { delete id; delete dims; return 0; }
	ast::Node *varType2 = tryVariableAsType(i);
	if (varType == 0) {
		if (varType2 == 0) {
			if (variableTypesAreEqual(varType, varType2)) {
				emit error(ErrorCodes::ecVariableTypeDefinedTwice, tr("Variable \"%1\" type defined twice"), i->codePoint());
				mStatus = ErrorButContinue;
			}
			else {
				emit warning(WarningCodes::wcVariableTypeDefinedTwice, tr("Variable \"%1\" type defined twice"), i->codePoint());
			}
		}
	}
	if (!varType) varType = varType2;
	if (!varType) varType = new ast::DefaultType(cp);

	ast::ArrayInitialization *init = new ast::ArrayInitialization(cp);
	init->setDimensions(dims);
	init->setIdentifier(id);
	init->setValueType(varType);
	return init;
}

bool Parser::expectLeftParenthese(Parser::TokIterator &i) {
	if (i->type() != Token::LeftParenthese) {
		emit error(ErrorCodes::ecExpectingLeftParenthese, tr("Expecting a left parenthese after \"%1\"").arg((i - 1)->toString()), i->codePoint());
		mStatus = Error;
		return false;
	}
	i++;
	return true;
}

bool Parser::expectRightParenthese(Parser::TokIterator &i) {
	if (i->type() != Token::RightParenthese) {
		emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting a right parenthese after \"%1\"").arg((i - 1)->toString()), i->codePoint());
		mStatus = Error;
		return false;
	}
	i++;
	return true;
}

bool Parser::expectLeftSquareBracket(Parser::TokIterator &i) {
	if (i->type() != Token::LeftSquareBracket) {
		emit error(ErrorCodes::ecExpectingLeftSquareBracket, tr("Expecting a left square bracket after \"%1\"").arg((i - 1)->toString()), i->codePoint());
		mStatus = Error;
		return false;
	}
	i++;
	return true;
}

bool Parser::expectRightSquareBracket(Parser::TokIterator &i){
	if (i->type() != Token::RightSquareBracket) {
		emit error(ErrorCodes::ecExpectingRightSquareBracket, tr("Expecting a right square bracket after \"%1\"").arg((i - 1)->toString()), i->codePoint());
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
			emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting right parenthese, got \"%1\"").arg(i->toString()), i->codePoint());
			mStatus = Error;
			return 0;
		}
		i++;
		ast::Node *varType2 = tryVariableAsType(i);
		if (varType != 0) {
			if (varType2 != 0) {
				if (variableTypesAreEqual(varType, varType2)) {
					emit error(ErrorCodes::ecVariableTypeDefinedTwice, tr("Variable \"%1\" type defined twice"), i->codePoint());
					mStatus = ErrorButContinue;
				}
				else {
					emit warning(WarningCodes::wcVariableTypeDefinedTwice, tr("Variable \"%1\" type defined twice"), i->codePoint());
				}
			}
		}
		else {
			varType = varType2;
		}
		if (!varType) varType = new ast::DefaultType(cp);

		ast::ArrayInitialization *arr = new ast::ArrayInitialization(cp);
		arr->setIdentifier(id);
		arr->setDimensions(dims);
		arr->setValueType(varType);
		return arr;
	}
	else {
		ast::Node *value = 0;
		if (!varType) varType = new ast::DefaultType(cp);
		if (i->type() == Token::opAssign) {
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
				emit error(ErrorCodes::ecExpectingRightSquareBracket, tr("Expecting a right parenthese"), i->codePoint());
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
			emit error(ErrorCodes::ecExpectingPrimaryExpression, tr("Expecting a primary type expression after \"%1\"").arg((i - 1)->toString()), i->codePoint());
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
		while ((i++)->type() == Token::Comma) ++dims;
		if (i->type() != Token::RightSquareBracket) {
			emit error(ErrorCodes::ecExpectingRightSquareBracket, tr("Expecting a right square bracket after ','"), i->codePoint());
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
	return expectArrayTypeDefinition(i);
}

void Parser::expectEndOfStatement(Parser::TokIterator &i) {
	if (i->isEndOfStatement()) {
		i++;
		return;
	}
	mStatus = Error;
	emit error(ErrorCodes::ecExpectingEndOfStatement, tr("Expecting end of line or ':', got \"%1\"").arg(i->toString()), i->codePoint());
	i++;
}

bool Parser::variableTypesAreEqual(ast::Node *a, ast::Node *b) {
	assert(a->type() == ast::Node::ntNamedType || a->type() == ast::Node::ntArrayType);
	assert(b->type() == ast::Node::ntNamedType || b->type() == ast::Node::ntArrayType);

	if (a->type() != b->type()) return false;
	if (a->type() == ast::Node::ntNamedType) {
		return static_cast<ast::NamedType*>(a)->identifier()->name() == static_cast<ast::NamedType*>(b)->identifier()->name();
	}
	else { // ast::Node::ntArrayType
		ast::ArrayType *arrTyA = static_cast<ast::ArrayType*>(a);
		ast::ArrayType *arrTyB = static_cast<ast::ArrayType*>(b);
		if (arrTyA->dimensions() != arrTyB->dimensions()) return false;
		return variableTypesAreEqual(arrTyA->parentType(), arrTyB->parentType());
	}
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


ast::Variable *Parser::expectVariable(Parser::TokIterator &i) {
	ast::Variable* var = tryVariable(i);
	if (!var) {
		emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting a variable, got \"%1\"").arg(i->toString()), i->codePoint());
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
		emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting an identifier, got \"%1\"").arg(i->toString()), i->codePoint());
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
	QList<ast::SelectCase*> cases;
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
			cases.append(c);
			continue;
		}
		else if (i->type() == Token::kDefault) {
			if (defaultCase != 0) {
				emit error(ErrorCodes::ecMultipleSelectDefaultCases, tr("Multiple default cases"), i->codePoint());
				mStatus = ErrorButContinue;
			}
			i++;
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			defaultCase = expectBlock(i);
			if (mStatus == Error) return 0;
		}
		else {
			emit error(ErrorCodes::ecExpectingEndSelect, tr("Expecting \"EndSelect\", \"Case\" or \"Default\", got \"%1\"").arg(i->toString()), i->codePoint());
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

	QList<ast::ArrayInitialization*> inits;
	do {
		ast::ArrayInitialization *arrInit = expectArrayInitialization(i);
		if (mStatus == Error) {
			qDeleteAll(inits);
			return 0;
		}
		inits.append(arrInit);
	} while ((i++)->type() == Token::Comma);

	ast::Redim *arr = new ast::Redim(cp);
	arr->setArrayInializations(inits);
	return arr;
}

ast::Node *Parser::tryDim(Parser::TokIterator &i) {
	if (i->type() == Token::kDim) {
		CodePoint cp = i->codePoint();
		i++;
		QList<ast::Node*> definitions;
		while (true) {
			ast::Node *def = expectVariableDefinitionOrArrayInitialization(i);
			if (mStatus == Error) return 0;

			definitions.append(def);

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
			emit error(ErrorCodes::ecExpectingEndOfStatement, tr("Expecting the end of the line, ':', or \"then\", got \"%1\"").arg(i->toString()), i->codePoint());
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
			emit error(ErrorCodes::ecExpectingEndIf, tr("Expecting \"EndIf\" for if-statement, which begins at %1,").arg(startCp.toString()), i->codePoint());
			mStatus = Error;
			return 0;
		}
	}
	else if (i->type() != Token::kEndIf) {
		emit error(ErrorCodes::ecExpectingEndIf, tr("Expecting \"EndIf\" for if-statement, which begins at %1,").arg(startCp.toString()), i->codePoint());
		mStatus = Error;
		return 0;
	}


	ast::IfStatement *ret = new ast::IfStatement(startCp, i->codePoint());
	ret->setElseBlock(elseBlock);
	ret->setCondition(condition);
	ret->setBlock(block);
	i++;
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
			emit error(ErrorCodes::ecExpectingWend, tr("Expecting \"Wend\" to end \"While\" on line %1, got %2").arg(QString::number(startCp.line()), i->toString()),i->codePoint());
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
			emit error(ErrorCodes::ecExpectingLeftParenthese, tr("Expecting '(' after a function name, got \"%1\"").arg(i->toString()), i->codePoint());
			mStatus = Error;
			return 0;
		}
		i++;

		ast::Node* args = 0;
		if (i->type() != Token::RightParenthese) {
			args = expectFunctionParameterList(i);
			if (mStatus == Error) return 0;
			if (i->type() != Token::RightParenthese) {
				emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting ')', got \"%1\"").arg(i->toString()), i->codePoint());
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
				emit warning(WarningCodes::wcFunctionReturnTypeDefinedTwice, tr("Function return type defined twice"), i->codePoint());
			}
			else {
				emit error(ErrorCodes::ecFunctionReturnTypeDefinedTwice, tr("Function return type defined twice"), i->codePoint());
				mStatus = ErrorButContinue;
			}
		}
		if (retType2) retType = retType2;
		if (!retType) {
			emit error(ErrorCodes::ecFunctionReturnTypeRequired, tr("Function return type required"), startCp);
			mStatus = ErrorButContinue;
		}

		expectEndOfStatement(i);
		ast::Node *block = expectBlock(i);
		if (i->type() != Token::kEndFunction) {
			emit error(ErrorCodes::ecExpectingEndFunction, tr("Expecting \"EndFunction\", got %1").arg(i->toString()), i->codePoint());
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
		case Token::opAssign:
			return ast::ExpressionNode::opAssign;
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

ast::Node *Parser::expectExpression(Parser::TokIterator &i) {
	return expectAssignementExpression(i);

}

ast::Node *Parser::expectAssignementExpression(Parser::TokIterator &i) {
	CodePoint cp1 = i->codePoint();
	ast::Node *first = expectLogicalOrExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opAssign) {
		CodePoint cp2 = i->codePoint();
		++i;
		ast::Node *second = expectLogicalOrExpression(i);
		if (mStatus == Error) { delete first; return 0; }
		ast::Expression *expr = new ast::Expression(ast::Expression::RightToLeft, cp1);
		expr->setFirstOperand(first);
		ast::ExpressionNode *exprNode = new ast::ExpressionNode(ast::ExpressionNode::opAssign, cp2);
		exprNode->setOperand(second);
		expr->appendOperation(exprNode);
		while (i->type() == Token::opAssign) {
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
	ast::Node *first = expectBitShiftExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opGreater || i->type() == Token::opLess || i->type() == Token::opGreaterEqual || i->type() == Token::opLessEqual) {
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
		while (i->type() == Token::opGreater || i->type() == Token::opLess || i->type() == Token::opGreaterEqual || i->type() == Token::opLessEqual) {
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
	ast::Node *first = expectAdditiveExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opShl || i->type() == Token::opShr || i->type() == Token::opSar) {
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
		while (i->type() == Token::opShl || i->type() == Token::opShr || i->type() == Token::opSar) {
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
	ast::Node *first = expectPowExpression(i);
	if (mStatus == Error) return 0;
	if (i->type() == Token::opMultiply || i->type() == Token::opDivide || i->type() == Token::opMod) {
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
		while (i->type() == Token::opMultiply || i->type() == Token::opDivide || i->type() == Token::opMod) {
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
					emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting a right parenthese, got \"%1\"").arg(i->toString()), i->codePoint());
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
					emit error(ErrorCodes::ecExpectingRightSquareBracket, tr("Expecting a right square bracket, got \"%1\"").arg(i->toString()), i->codePoint());
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
			bool success;
			int val = ('-' + i->toString()).toInt(&success);
			if (!success) {
				emit error(ErrorCodes::ecCantParseInteger, tr("Cannot parse an integer \"%1\"").arg('-' + i->toString()), i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer(val, i->codePoint());
			i++;
			return intN;
		}
		case Token::IntegerHex: {
			bool success;
			int val = ('-' + i->toString()).toInt(&success,16);
			if (!success) {
				emit error(ErrorCodes::ecCantParseInteger, tr("Cannot parse integer \"%1\"").arg('-' + i->toString()), i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer(val, i->codePoint());
			i++;
			return intN;
		}

		case Token::Float: {
			bool success;
			float val;
			if (*i->begin() == '.') { //leading dot .13123
				val = ("-0" + i->toString()).toFloat(&success);
			}
			else if (*(i->end() - 1) == '.') { //Ending dot 1231.
				val = ('-' + i->toString() + '0').toFloat(&success);
			}
			else {
				val = ('-' + i->toString()).toFloat(&success);
			}
			if (!success) {
				emit error(ErrorCodes::ecCantParseFloat, tr("Cannot parse float \"%1\"").arg('-' + i->toString()), i->codePoint());
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
				emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting a right parenthese, got \"%1\"").arg(i->toString()), i->codePoint());
				mStatus = Error;
				return 0;
			}
			i++;
			return expr;
		}
		case Token::Integer: {
			bool success;
			int val = i->toString().toInt(&success);
			if (!success) {
				emit error(ErrorCodes::ecCantParseInteger, tr("Cannot parse an integer \"%1\"").arg(i->toString()), i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer(val, i->codePoint());
			i++;
			return intN;
		}
		case Token::IntegerHex: {
			bool success;
			int val = i->toString().toInt(&success,16);
			if (!success) {
				emit error(ErrorCodes::ecCantParseInteger, tr("Cannot parse integer \"%1\"").arg(i->toString()), i->codePoint());
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer(val, i->codePoint());
			i++;
			return intN;
		}

		case Token::Float: {
			bool success;
			float val;
			if (*i->begin() == '.') { //leading dot .13123
				val = ('0' + i->toString()).toFloat(&success);
			}
			else if (*(i->end() - 1) == '.') { //Ending dot 1231.
				val = (i->toString() + '0').toFloat(&success);
			}
			else {
				val = i->toString().toFloat(&success);
			}
			if (!success) {
				emit error(ErrorCodes::ecCantParseFloat, tr("Cannot parse float \"%1\"").arg(i->toString()), i->codePoint());
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
			emit error(ErrorCodes::ecExpectingPrimaryExpression, tr("Expecting a primary expression, got \"%1\"").arg(i->toString()), i->codePoint());
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
	QList<ast::Node*> definitions;
	while (true) {
		ast::Node *def = expectVariableDefinitionOrArrayInitialization(i);
		if (mStatus == Error) return 0;

		definitions.append(def);

		if (i->type() != Token::Comma) break;
		i++;
	}
	if (definitions.size() == 1) {
		return definitions.first();
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
	if (i->type() == Token::opAssign) {
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
		emit error(ErrorCodes::ecExpectingEndOfRepeat, tr("Expecting \"Until\" or \"Forever\", got \"%1\""), i->codePoint());
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

		if (i->type() == Token::opAssign) {
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
					emit error(ErrorCodes::ecExpectingNext, tr("Expecting \"Next\" to end For-Each block starting at line %1").arg(startCp.line()), i->codePoint());
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
				if (nextV) {
					emit warning(WarningCodes::wcNextVariableIgnored, tr("The variable name after \"Next\" is ignored"), nextVCp);
					delete nextV;
				}
				return forEach;
			}

			//For-To
			i = start;
			delete part1;
			part1 = expectExpression(i);
			if (mStatus == Error) { return 0;}
		}

		if (i->type() != Token::kTo) {
			emit error(ErrorCodes::ecExpectingTo, tr("Expecting \"To\" after \"%1\"").arg((i - 1)->toString()), i->codePoint());
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
			emit error(ErrorCodes::ecExpectingNext, tr("Expecting \"Next\" to end For-Each block starting at line %1").arg(startCp.line()), i->codePoint());
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
			emit warning(WarningCodes::wcNextVariableIgnored, tr("The variable name after \"Next\" is ignored"), nextVCp);
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



