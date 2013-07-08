#include "parser.h"
#include "errorcodes.h"
#include <assert.h>

Parser::Parser():
	mStatus(Ok)
{
}

typedef ast::Node *(Parser::*BlockParserFunction)(Parser::TokIterator &);
static BlockParserFunction blockParsers[] =  {
	&Parser::tryGotoGosubAndLabel,
	&Parser::tryDim,
	&Parser::tryRedim,
	&Parser::tryIfStatement,
	&Parser::tryWhileStatement,
	&Parser::tryRepeatStatement,
	&Parser::tryForStatement,
	&Parser::tryReturn,
	&Parser::trySelectStatement,
	&Parser::tryAssignmentExpression,
	&Parser::tryFunctionOrCommandCallOrArraySubscriptAssignment,
};
static const int blockParserCount = 11;
ast::Block Parser::expectBlock(Parser::TokIterator &i) {
	ast::Block block;
	while (true) {
		if (i->isEndOfStatement()) {
			i++;
			if (i->mType == Token::EndOfTokens) {
				i--;
				return block;
			}
			continue;
		}
		ast::Node * n;
		for (int p = 0; p < blockParserCount; p++) {
			n =	(this->*(blockParsers[p]))(i);
			if (mStatus == Error) return ast::Block();
			if (n) {
				expectEndOfStatement(i);
				if (mStatus == Error) return ast::Block();
				break;
			}
		}
		if (n) {
			block.append(n);
		}
		else {
			break;
		}
	}
	return block;
}

ast::Block Parser::expectInlineBlock(Parser::TokIterator &i) {
	ast::Block block;
	int line = i->mLine;
	while (i->mLine == line) {
		if (i->isEndOfStatement()) {
			i++;
			if (i->mType == Token::EndOfTokens) {
				i--;
				return block;
			}
			continue;
		}
		ast::Node * n;
		for (int p = 0; p < blockParserCount; p++) {
			n = (this->*(blockParsers[p]))(i);
			if (mStatus == Error) return ast::Block();
			if (n) {
				if (i->mType == Token::EOL || i->mType == Token::kElse || i->mType == Token::kElseIf) { // WTF CB?
					block.append(n);
					return block;
				}
				else {
					expectEndOfStatement(i);
					if (mStatus == Error) return ast::Block();
				}
				break;
			}
		}
		if (n) {
			block.append(n);
		}
		else {
			break;
		}
	}
	return block;
}

ast::Program *Parser::parse(const QList<Token> &tokens, const Settings &settings) {
	mSettings = settings;

	ast::Program *program = new ast::Program;
	TokIterator i = tokens.begin();
	while (i->mType != Token::EndOfTokens) {
		if (i->isEndOfStatement()) {
			i++;
			continue;
		}
		ast::TypeDefinition *type = tryTypeDefinition(i);
		if (mStatus == Error) return 0;
		if (type) {
			program->mTypes.append(type);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}

		ast::FunctionDefinition *func = tryFunctionDefinition(i);
		if (mStatus == Error) return 0;
		if (func) {
			program->mFunctions.append(func);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}

		ast::GlobalDefinition *global = tryGlobalDefinition(i);
		if (mStatus == Error) return 0;
		if (global) {
			program->mGlobals.append(global);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}

		ast::ConstDefinition *constant = tryConstDefinition(i);
		if (mStatus == Error) return 0;
		if (constant) {
			program->mConstants.append(constant);
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			continue;
		}
		ast::Node * n;
		for (int p = 0; p < blockParserCount; p++) {
			n =	(this->*(blockParsers[p]))(i);
			if (mStatus == Error) return program;
			if (n) {
				expectEndOfStatement(i);
				if (mStatus == Error) return program;
				break;
			}
		}
		if (n) {
			program->mMainBlock.append(n);
		}
		else {
			emit error(ErrorCodes::ecUnexpectedToken, tr("Unexpected token \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return program;
		}
	}

	return program;
}

ast::ConstDefinition *Parser::tryConstDefinition(Parser::TokIterator &i) {
	if (i->mType == Token::kConst) {
		int line = i->mLine;
		QFile *file = i->mFile;
		i++;
		QString name = expectIdentifier(i);
		ast::Variable::VarType varType = tryVarType(i);
		if (i->mType != Token::opEqual) {
			emit error(ErrorCodes::ecExpectingAssignment, tr("Expecting '=' after constant name, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		i++;
		ast::Node *value = expectExpression(i);
		if (mStatus == Error) return 0;
		ast::ConstDefinition *ret = new ast::ConstDefinition;
		ret->mName = name;
		ret->mValue = value;
		ret->mVarType = varType;
		ret->mFile = file;
		ret->mLine = line;
		return ret;
	}
	return 0;
}

ast::GlobalDefinition *Parser::tryGlobalDefinition(Parser::TokIterator &i) {
	if (i->mType == Token::kGlobal) {
		int line = i->mLine;
		QFile *file = i->mFile;
		i++;
		ast::Variable *var = expectVariableOrTypePtrDefinition(i);
		if (mStatus == Error) return 0;
		ast::GlobalDefinition *ret = new ast::GlobalDefinition;
		ret->mFile = file;
		ret->mLine = line;
		ret->mDefinitions.append(var);
		while (i->mType == Token::Comma) {
			i++;
			ast::Variable *var = expectVariableOrTypePtrDefinition(i);
			if (mStatus == Error) return 0;
			ret->mDefinitions.append(var);
		}
		return ret;
	}

	//Not global
	return 0;
}

ast::Variable::VarType Parser::tryVarAsType(Parser::TokIterator &i) {
	if (i->mType == Token::kAs) {
		i++;
		switch (i->mType) {
			case Token::kInteger:
				i++;
				return ast::Variable::Integer;
			case Token::kFloat:
				i++;
				return ast::Variable::Float;
			case Token::kString:
				i++;
				return ast::Variable::String;
			case Token::kShort:
				i++;
				return ast::Variable::Short;
			case Token::kByte:
				i++;
				return ast::Variable::Byte;
			default:
				mStatus = Error;
				emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting variable type after \"as\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				return ast::Variable::Invalid;
		}
	}
	return ast::Variable::Default;
}


ast::Variable::VarType Parser::tryVarType(Parser::TokIterator &i)
{
	switch(i->mType) {
		case Token::kAs:
			i++;
			switch (i->mType) {
				case Token::kInteger:
					i++;
					return ast::Variable::Integer;
				case Token::kFloat:
					i++;
					return ast::Variable::Float;
				case Token::kString:
					i++;
					return ast::Variable::String;
				case Token::kShort:
					i++;
					return ast::Variable::Short;
				case Token::kByte:
					i++;
					return ast::Variable::Byte;
				default:
					mStatus = Error;
					emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting variable type after \"as\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
					return ast::Variable::Invalid;
			}

		case Token::IntegerTypeMark:
			i++;
			return ast::Variable::Integer;
		case Token::FloatTypeMark:
			i++;
			return ast::Variable::Float;
		case Token::StringTypeMark:
			i++;
			return ast::Variable::String;
		default:
			return ast::Variable::Default;
	}
}

ast::Variable::VarType Parser::tryVarTypeSymbol(Parser::TokIterator &i)
{
	switch(i->mType) {
		case Token::IntegerTypeMark:
			i++;
			return ast::Variable::Integer;
		case Token::FloatTypeMark:
			i++;
			return ast::Variable::Float;
		case Token::StringTypeMark:
			i++;
			return ast::Variable::String;
		default:
			return ast::Variable::Default;
	}
}

ast::Node *Parser::tryReturn(Parser::TokIterator &i) {
	if (i->mType == Token::kReturn) {
		int line = i->mLine;
		QFile *file = i->mFile;
		i++;
		ast::Node *r = 0;
		if (!i->isEndOfStatement()) {
			r = expectExpression(i);
		}
		if (mStatus == Error) return 0;
		ast::Return *ret = new ast::Return;
		ret->mValue = r;
		ret->mFile = file;
		ret->mLine = line;
		return ret;
	}
	return 0;
}

ast::TypeDefinition *Parser::tryTypeDefinition(Parser::TokIterator &i) {
	if (i->mType == Token::kType) {
		QFile *file = i->mFile;
		int line = i->mLine;
		i++;
		QString name = expectIdentifier(i);
		if (mStatus == Error) return 0;
		expectEndOfStatement(i);
		if (mStatus == Error) return 0;
		QList<QPair<int, ast::Variable*> > fields;
		while (i->isEndOfStatement()) i++;
		while (i->mType == Token::kField) {
			int line = i->mLine;
			i++;
			ast::Variable *field = expectVariableOrTypePtrDefinition(i);
			if (mStatus == Error) return 0;
			fields.append(QPair<int, ast::Variable*> (line, field));
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;

			while (i->isEndOfStatement()) i++;
		}
		if (i->mType != Token::kEndType) {
			emit error(ErrorCodes::ecExpectingEndType, tr("Expecting \"EndType\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		i++;

		ast::TypeDefinition *ret = new ast::TypeDefinition;
		ret->mFields = fields;
		ret->mFile = file;
		ret->mLine = line;
		ret->mName = name;
		return ret;
	}
	return 0;
}

QString Parser::expectIdentifier(Parser::TokIterator &i) {
	if (i->mType == Token::Identifier) {
		QString ret = i->toString();
		i++;
		return ret;
	}

	emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting identifier, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
	mStatus = Error;
	return QString();
}

void Parser::expectEndOfStatement(Parser::TokIterator &i) {
	if (i->isEndOfStatement()) {
		i++;
		return;
	}
	mStatus = Error;
	emit error(ErrorCodes::ecExpectingEndOfStatement, tr("Expecting end of line or ':', got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
	i++;
}


ast::Variable *Parser::expectVariableOrTypePtrDefinition(Parser::TokIterator &i) {
	ast::Variable * var = tryVariableOrTypePtrDefinition(i);
	if (!var) {
		emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting variable, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
	}
	return var;
}

void Parser::expectVariableOrTypePtrDefinition(ast::Variable *var, TokIterator &i) {
	var->mName = expectIdentifier(i);
	if (mStatus == Error) return;

	if (i->mType == Token::opTypePtrType) {
		i++;
		if (i->mType != Token::Identifier) {
			emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting type name after '.', got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return;
		}
		var->mVarType = ast::Variable::TypePtr;
		var->mTypeName = i->toString();
		i++;
		return;
	}
	else {
		var->mVarType = tryVarType(i);
		return;
	}
}

ast::Variable *Parser::tryVariableOrTypePtrDefinition(Parser::TokIterator &i) {
	if (i->mType != Token::Identifier) {
		return 0;
	}
	QString name = i->toString();
	i++;
	if (i->mType == Token::opTypePtrType) {
		i++;
		if (i->mType != Token::Identifier) {
			emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting type name after '.', got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		ast::Variable *ret = new ast::Variable;
		ret->mName = name;
		ret->mVarType = ast::Variable::TypePtr;
		ret->mTypeName = i->toString();
		i++;
		return ret;
	}
	else {
		ast::Variable::VarType type = tryVarType(i);
		if (mStatus == Error) return 0;
		ast::Variable *ret = new ast::Variable;
		ret->mName = name;
		ret->mVarType = type;
		return ret;
	}
}

ast::Node *Parser::trySelectStatement(Parser::TokIterator &i) {
	if (i->mType != Token::kSelect) return 0;
	//Select
	int startLine = i->mLine;
	QFile *file = i->mFile;
	i++;
	ast::Variable *var = expectVariableOrTypePtrDefinition(i);
	if (mStatus == Error) return 0;
	expectEndOfStatement(i);
	if (mStatus == Error) return 0;
	QList<ast::Case> cases;
	ast::Block defaultCase;
	while (i->mType != Token::kEndSelect) {
		if (i->isEndOfStatement()) {
			i++;
			continue;
		}
		if (i->mType == Token::kCase) {
			ast::Case c;
			c.mLine = i->mLine;
			c.mFile = i->mFile;
			i++;
			c.mCase = expectExpression(i);
			if (mStatus == Error) return 0;
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			c.mBlock = expectBlock(i);
			if (mStatus == Error) return 0;
			cases.append(c);
			continue;
		}
		else if (i->mType == Token::kDefault) {
			if (!defaultCase.isEmpty()) {
				emit error(ErrorCodes::ecMultipleSelectDefaultCases, tr("Multiple default cases"), i->mLine, i->mFile);
				mStatus = ErrorButContinue;
			}
			i++;
			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			defaultCase = expectBlock(i);
			if (mStatus == Error) return 0;
		}
		else {
			emit error(ErrorCodes::ecExpectingEndSelect, tr("Expecting \"EndSelect\", \"Case\" or \"Default\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
	}
	ast::SelectStatement *ret = new ast::SelectStatement;
	ret->mCases = cases;
	ret->mDefault = defaultCase;
	ret->mVariable = var;
	ret->mFile = file;
	ret->mStartLine = startLine;
	ret->mEndLine = i->mLine;
	i++;
	return ret;
}


ast::Node *Parser::tryGotoGosubAndLabel(Parser::TokIterator &i) {
	switch (i->mType) {
		case Token::kGoto: {
			ast::Goto *ret = new ast::Goto;
			ret->mFile = i->mFile;
			ret->mLine = i->mLine;
			i++;
			ret->mLabel = expectIdentifier(i);
			if (mStatus == Error) return 0;
			return ret;
		}
		case Token::kGosub: {
			ast::Gosub *ret = new ast::Gosub;
			ret->mFile = i->mFile;
			ret->mLine = i->mLine;
			i++;
			ret->mLabel = expectIdentifier(i);
			if (mStatus == Error) return 0;
			return ret;
		}
		case Token::Label: {
			ast::Label *label = new ast::Label;
			label->mFile = i->mFile;
			label->mLine = i->mLine;
			label->mName = i->toString();
			i++;
			return label;
		}
		default:
			return 0;
	}
}

ast::Node *Parser::tryRedim(Parser::TokIterator &i) {
	if (i->mType != Token::kRedim) return 0;
	int line = i->mLine;
	QFile *file = i->mFile;
	i++;
	QString name = expectIdentifier(i);
	ast::Variable::VarType varType = tryVarTypeSymbol(i);
	if (mStatus == Error) return 0;
	if (i->mType != Token::LeftParenthese) {
		emit error(ErrorCodes::ecExpectingLeftParenthese, tr("Expecting left parenthese, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
		mStatus = Error;
		return 0;
	}
	i++;

	QList<ast::Node*> dimensions;
	dimensions.append(expectExpression(i));
	if (mStatus == Error) return 0;
	while (i->mType == Token::Comma) {
		i++;
		dimensions.append(expectExpression(i));
		if (mStatus == Error) return 0;
	}

	if (i->mType != Token::RightParenthese) {
		emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting right parenthese, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
		mStatus = Error;
		return 0;
	}
	i++;

	if (varType == ast::Variable::Default) {
		varType = tryVarAsType(i);
		if (mStatus == Error) return 0;
	}
	ast::Redim *arr = new ast::Redim;
	arr->mLine = line;
	arr->mFile = file;
	arr->mName = name;
	arr->mType = varType;
	arr->mDimensions = dimensions;
	return arr;
}

ast::Node *Parser::tryDim(Parser::TokIterator &i) {
	Parser::TokIterator begin = i;
	if (i->mType == Token::kDim) {
		QFile *file = i->mFile;
		int line = i->mLine;
		i++;
		if (i->mType != Token::Identifier) {
			emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting variable name after \"Dim\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		QString name = i->toString();
		i++;
		ast::Variable::VarType varType = tryVarTypeSymbol(i);
		if (mStatus == Error) return 0;
		if (i->mType == Token::LeftParenthese) { //Array
			i++;

			QList<ast::Node*> dimensions;
			dimensions.append(expectExpression(i));
			if (mStatus == Error) return 0;
			while (i->mType == Token::Comma) {
				i++;
				dimensions.append(expectExpression(i));
				if (mStatus == Error) return 0;
			}

			if (i->mType != Token::RightParenthese) {
				emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting right parenthese, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;

			if (varType == ast::Variable::Default) {
				varType = tryVarAsType(i);
				if (mStatus == Error) return 0;
			}
			ast::ArrayDefinition *arr = new ast::ArrayDefinition;
			arr->mLine = line;
			arr->mFile = file;
			arr->mName = name;
			arr->mType = varType;
			arr->mDimensions = dimensions;
			return arr;
		}
		else {
			QString typeName;
			if (varType == ast::Variable::Default) {
				varType = tryVarAsType(i);
				if (mStatus == Error) return 0;
				if (varType == ast::Variable::Default) {
					if (i->mType == Token::opTypePtrType) {
						i++;
						varType = ast::Variable::TypePtr;
						if (i->mType != Token::Identifier) {
							emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting type name after '.', got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
							mStatus = Error;
							return 0;
						}
						typeName = i->toString();
						i++;
					}
				}
			}
			ast::Variable *var = new ast::Variable;
			var->mName = name;
			var->mVarType = varType;
			var->mTypeName = typeName;
			ast::VariableDefinition *ret = new ast::VariableDefinition;
			ret->mLine = line;
			ret->mFile = file;
			ret->mDefinitions.append(var);
			while (i->mType == Token::Comma) {
				i++;
				ast::Variable *var = expectVariableOrTypePtrDefinition(i);
				if (mStatus == Error) return 0;
				ret->mDefinitions.append(var);
			}
			return ret;
		}
	}
	return 0;
}

ast::Node *Parser::tryIfStatement(Parser::TokIterator &i) { //FINISH
	if (i->mType == Token::kIf) {
		int line = i->mLine;
		QFile *file = i->mFile;
		i++;
		ast::Node *condition = expectExpression(i);
		if (mStatus == Error) return 0;
		if (i->mType == Token::kThen) {
			i++;
			if (!i->isEndOfStatement()) { //Inline if
				ast::Block block = expectInlineBlock(i);
				if (mStatus == Error) return 0;
				ast::IfStatement *ret = new ast::IfStatement;
				ret->mLine = line;
				ret->mFile = file;
				ret->mCondition = condition;
				ret->mIfTrue = block;
				if (i->mType == Token::kElseIf) {
					ast::Node *elseIf = expectElseIfStatement(i);
					if (mStatus == Error) return 0;
					ast::Block elseBlock;
					elseBlock.append(elseIf);
					ret->mElse = elseBlock;
					return ret;
				}
				if (i->mType == Token::kElse) {
					i++;
					ast::Block elseBlock = expectInlineBlock(i);
					if (mStatus == Error) return 0;
					ret->mElse = elseBlock;
				}
				return ret;
			}
			i++;
		}
		else {
			if (!i->isEndOfStatement()) {
				emit error(ErrorCodes::ecExpectingEndOfStatement, tr("Expecting end of line, ':', or \"then\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;
		}

		ast::Block block = expectBlock(i);
		if (mStatus == Error) return 0;
		ast::IfStatement *ret = new ast::IfStatement;
		ret->mLine = line;
		ret->mFile = file;
		ret->mCondition = condition;
		ret->mIfTrue = block;
		if (i->mType == Token::kElseIf) {
			ast::Node *elseIf = expectElseIfStatement(i);
			if (mStatus == Error) return 0;
			ast::Block elseBlock;
			elseBlock.append(elseIf);
			ret->mElse = elseBlock;
			return ret;
		}
		if (i->mType == Token::kElse) {
			i++;
			ast::Block elseBlock = expectBlock(i);
			if (mStatus == Error) return 0;
			ret->mElse = elseBlock;
		}
		if (i->mType != Token::kEndIf) {
			emit error(ErrorCodes::ecExpectingEndIf, tr("Expecting \"EndIf\" for if-statement, which begin at line %1,").arg(QString::number(line)), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		i++;
		return ret;
	}

	return 0;
}

ast::Node *Parser::expectElseIfStatement(Parser::TokIterator &i) {
	assert(i->mType == Token::kElseIf);
	int line = i->mLine;
	QFile *file = i->mFile;
	i++;
	ast::Node *condition = expectExpression(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::kThen) {
		i++;
		if (!i->isEndOfStatement()) { //Inline if
			ast::Block block = expectInlineBlock(i);
			if (mStatus == Error) return 0;
			ast::IfStatement *ret = new ast::IfStatement;
			ret->mLine = line;
			ret->mFile = file;
			ret->mCondition = condition;
			ret->mIfTrue = block;
			if (i->mType == Token::kElseIf) {
				ast::Node *elseIf = expectElseIfStatement(i);
				if (mStatus == Error) return 0;
				ast::Block elseBlock;
				elseBlock.append(elseIf);
				ret->mElse = elseBlock;
				return ret;
			}
			if (i->mType == Token::kElse) {
				i++;
				ast::Block elseBlock = expectInlineBlock(i);
				if (mStatus == Error) return 0;
				ret->mElse = elseBlock;
			}
			return ret;
		}
		i++;
	}
	else {
		if (!i->isEndOfStatement()) {
			emit error(ErrorCodes::ecExpectingEndOfStatement, tr("Expecting end of line, ':', or \"then\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		i++;
	}

	ast::Block block = expectBlock(i);
	if (mStatus == Error) return 0;
	ast::IfStatement *ret = new ast::IfStatement;
	ret->mLine = line;
	ret->mFile = file;
	ret->mCondition = condition;
	ret->mIfTrue = block;
	if (i->mType == Token::kElseIf) {
		ast::Node *elseIf = expectElseIfStatement(i);
		if (mStatus == Error) return 0;
		ast::Block elseBlock;
		elseBlock.append(elseIf);
		ret->mElse = elseBlock;
		return ret;
	}
	if (i->mType == Token::kElse) {
		i++;
		ast::Block elseBlock = expectBlock(i);
		if (mStatus == Error) return 0;
		ret->mElse = elseBlock;
	}
	if (i->mType != Token::kEndIf) {
		emit error(ErrorCodes::ecExpectingEndIf, tr("Expecting \"EndIf\" for if-statement, which begin at line %1").arg(QString::number(line)), i->mLine, i->mFile);
		mStatus = Error;
		return 0;
	}
	i++;
	return ret;
}

ast::Node *Parser::tryWhileStatement(Parser::TokIterator &i) {
	if (i->mType == Token::kWhile) {
		int startLine = i->mLine;
		QFile *file = i->mFile;
		i++;

		ast::Node *cond = expectExpression(i);
		if (mStatus == Error) return 0;

		expectEndOfStatement(i);
		if (mStatus == Error) return 0;

		ast::Block block = expectBlock(i);
		if (mStatus == Error) return 0;

		if (i->mType != Token::kWend) {
			emit error(ErrorCodes::ecExpectingWend, tr("Expecting \"Wend\" to end \"While\" on line %1, got %2").arg(QString::number(startLine), i->toString()),i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		int endLine = i->mLine;
		i++;

		ast::WhileStatement *ret = new ast::WhileStatement;
		ret->mStartLine = startLine;
		ret->mEndLine = endLine;
		ret->mFile = file;
		ret->mBlock = block;
		ret->mCondition = cond;
		return ret;
	}
	return 0;
}


ast::FunctionDefinition *Parser::tryFunctionDefinition(Parser::TokIterator &i) {
	if (i->mType == Token::kFunction) {
		int line = i->mLine;
		QFile *file = i->mFile;
		i++;
		QString functionName = expectIdentifier(i);
		if (mStatus == Error) return 0;

		ast::Variable::VarType retType = tryVarTypeSymbol(i);
		if (mStatus == Error) return 0;

		if (i->mType != Token::LeftParenthese) {
			emit error(ErrorCodes::ecExpectingLeftParenthese, tr("Expecting '(' after a function name, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		i++;

		QList<ast::FunctionParametreDefinition> params;
		if (i->mType != Token::RightParenthese) {
			ast::FunctionParametreDefinition param = expectFunctionParametreDefinition(i);
			if (mStatus == Error) return 0;
			params.append(param);
			while (i->mType == Token::Comma) {
				i++;
				param = expectFunctionParametreDefinition(i);
				if (mStatus == Error) return 0;
				params.append(param);
			}
		}
		if (i->mType != Token::RightParenthese) {
			emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting ')', got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		i++;
		ast::Variable::VarType retType2 = tryVarAsType(i);
		if (mStatus == Error) return 0;
		if (retType != ast::Variable::Default && retType2 != ast::Variable::Default) {
			if (retType == retType2) {
				emit warning(ErrorCodes::ecFunctionReturnTypeDefinedTwice, tr("Function return type defined twice"), i->mLine, i->mFile);
			}
			else {
				emit error(ErrorCodes::ecFunctionReturnTypeDefinedTwice, tr("Function return type defined twice"), i->mLine, i->mFile);
				mStatus = ErrorButContinue;
			}
		}
		if (retType2 != ast::Variable::Default) retType = retType2;
		if (retType == ast::Variable::Default) {
			emit error(ErrorCodes::ecFunctionReturnTypeRequired, tr("Function return type required"),i->mLine, i->mFile);
			mStatus = ErrorButContinue;
		}

		expectEndOfStatement(i);
		ast::Block block = expectBlock(i);
		if (i->mType != Token::kEndFunction) {
			emit error(ErrorCodes::ecExpectingEndFunction, tr("Expecting \"EndFunction\", got %1").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		i++;
		ast::FunctionDefinition *func = new ast::FunctionDefinition;
		func->mBlock = block;
		func->mName = functionName;
		func->mParams = params;
		func->mRetType = retType;
		func->mLine = line;
		func->mFile = file;
		return func;
	}
	return 0;
}

static ast::Operator tokenTypeToOperator(Token::Type t) {
	switch (t) {
		case Token::opEqual:
			return ast::opEqual;
		case Token::opNotEqual:
			return ast::opNotEqual;
		case Token::opGreater:
			return ast::opGreater;
		case Token::opLess:
			return ast::opLess;
		case Token::opGreaterEqual:
			return ast::opGreaterEqual;
		case Token::opLessEqual:
			return ast::opLessEqual;
		case Token::opPlus:
			return ast::opPlus;
		case Token::opMinus:
			return ast::opMinus;
		case Token::opMultiply:
			return ast::opMultiply;
		case Token::opPower:
			return ast::opPower;
		case Token::opMod:
			return ast::opMod;
		case Token::opShl:
			return ast::opShl;
		case Token::opShr:
			return ast::opShr;
		case Token::opSar:
			return ast::opSar;
		case Token::opDivide:
			return ast::opDivide;
		case Token::opAnd:
			return ast::opAnd;
		case Token::opOr:
			return ast::opOr;
		case Token::opXor:
			return ast::opXor;
		case Token::opNot:
			return ast::opNot;
		default:
			return ast::opInvalid;
	}
}

ast::Node *Parser::expectExpression(Parser::TokIterator &i) {
	return expectLogicalOrExpression(i);

}

ast::Node *Parser::expectLogicalOrExpression(Parser::TokIterator &i) {
	ast::Node *first = expectLogicalAndExpression(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::opOr || i->mType == Token::opXor) {
		ast::Operator op = tokenTypeToOperator(i->mType);
		i++;
		ast::Node *second = expectLogicalAndExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression;
		expr->mFirst = first;
		expr->mRest.append(ast::Operation(op, second));
		while (i->mType == Token::opOr || i->mType == Token::opXor) {
			ast::Operator op = tokenTypeToOperator(i->mType);
			i++;
			ast::Node *ep = expectLogicalAndExpression(i);
			if (mStatus == Error) return 0;
			expr->mRest.append(ast::Operation(op, ep));
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectLogicalAndExpression(Parser::TokIterator &i) {
	ast::Node *first = expectEqualityExpression(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::opAnd) {
		i++;
		ast::Node *second = expectEqualityExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression;
		expr->mFirst = first;
		expr->mRest.append(ast::Operation(ast::opAnd, second));
		while (i->mType == Token::opAnd) {
			i++;
			ast::Node *ep = expectEqualityExpression(i);
			if (mStatus == Error) return 0;
			expr->mRest.append(ast::Operation(ast::opAnd, ep));
		}
		return expr;
	}
	else {
		return first;
	}
}


ast::Node *Parser::expectEqualityExpression(Parser::TokIterator &i) {
	ast::Node *first = expectRelativeExpression(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::opEqual || i->mType == Token::opNotEqual) {
		ast::Operator op = tokenTypeToOperator(i->mType);
		i++;
		ast::Node *second = expectRelativeExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression;
		expr->mFirst = first;
		expr->mRest.append(ast::Operation(op, second));
		while (i->mType == Token::opEqual || i->mType == Token::opNotEqual) {
			op = tokenTypeToOperator(i->mType);
			i++;
			ast::Node *ep = expectRelativeExpression(i);
			if (mStatus == Error) return 0;
			expr->mRest.append(ast::Operation(op, ep));
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectRelativeExpression(Parser::TokIterator &i) {
	ast::Node *first = expectBitShiftExpression(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::opGreater || i->mType == Token::opLess || i->mType == Token::opGreaterEqual || i->mType == Token::opLessEqual) {
		ast::Operator op = tokenTypeToOperator(i->mType);
		i++;
		ast::Node *second = expectBitShiftExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression;
		expr->mFirst = first;
		expr->mRest.append(ast::Operation(op, second));
		while (i->mType == Token::opGreater || i->mType == Token::opLess || i->mType == Token::opGreaterEqual || i->mType == Token::opLessEqual) {
			op = tokenTypeToOperator(i->mType);
			i++;
			ast::Node *ep = expectBitShiftExpression(i);
			if (mStatus == Error) return 0;
			expr->mRest.append(ast::Operation(op, ep));
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectBitShiftExpression(Parser::TokIterator &i) {
	ast::Node *first = expectAdditiveExpression(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::opShl || i->mType == Token::opShr || i->mType == Token::opSar) {
		ast::Operator op = tokenTypeToOperator(i->mType);
		i++;
		ast::Node *second = expectAdditiveExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression;
		expr->mFirst = first;
		expr->mRest.append(ast::Operation(op, second));
		while (i->mType == Token::opShl || i->mType == Token::opShr || i->mType == Token::opSar) {
			op = tokenTypeToOperator(i->mType);
			i++;
			ast::Node *ep = expectAdditiveExpression(i);
			if (mStatus == Error) return 0;
			expr->mRest.append(ast::Operation(op, ep));
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectAdditiveExpression(Parser::TokIterator &i) {
	ast::Node *first = expectMultiplicativeExpression(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::opPlus || i->mType == Token::opMinus) {
		ast::Operator op = tokenTypeToOperator(i->mType);
		i++;
		ast::Node *second = expectMultiplicativeExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression;
		expr->mFirst = first;
		expr->mRest.append(ast::Operation(op, second));
		while (i->mType == Token::opPlus || i->mType == Token::opMinus) {
			op = tokenTypeToOperator(i->mType);
			i++;
			ast::Node *ep = expectMultiplicativeExpression(i);
			if (mStatus == Error) return 0;
			expr->mRest.append(ast::Operation(op, ep));
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectMultiplicativeExpression(Parser::TokIterator &i) {
	ast::Node *first = expectPowExpression(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::opMultiply || i->mType == Token::opDivide || i->mType == Token::opMod) {
		ast::Operator op = tokenTypeToOperator(i->mType);
		i++;
		ast::Node *second = expectPowExpression(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression;
		expr->mFirst = first;
		expr->mRest.append(ast::Operation(op, second));
		while (i->mType == Token::opMultiply || i->mType == Token::opDivide || i->mType == Token::opMod) {
			op = tokenTypeToOperator(i->mType);
			i++;
			ast::Node *ep = expectPowExpression(i);
			if (mStatus == Error) return 0;
			expr->mRest.append(ast::Operation(op, ep));
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectPowExpression(Parser::TokIterator &i) {
	ast::Node *first = expectUnaryExpession(i);
	if (mStatus == Error) return 0;
	if (i->mType == Token::opPower) {
		i++;
		ast::Node *second = expectUnaryExpession(i);
		if (mStatus == Error) return 0;
		ast::Expression *expr = new ast::Expression;
		expr->mFirst = first;
		expr->mRest.append(ast::Operation(ast::opPower, second));
		while (i->mType == Token::opPower) {
			i++;
			ast::Node *ep = expectUnaryExpession(i);
			if (mStatus == Error) return 0;
			expr->mRest.append(ast::Operation(ast::opPower, ep));
		}
		return expr;
	}
	else {
		return first;
	}
}

ast::Node *Parser::expectUnaryExpession(Parser::TokIterator &i) {
	switch (i->mType) {
		case Token::opPlus:
		case Token::opMinus:
		case Token::opNot: {
			ast::Operator op = tokenTypeToOperator(i->mType);
			i++;
			ast::Node *pri = expectPrimaryExpression(i);
			if (mStatus == Error) return 0;
			ast::Unary *unary = new ast::Unary;
			unary->mOperator = op;
			unary->mOperand = pri;
			return unary;
		}
		default:
			return expectPrimaryExpression(i);
	}
}

ast::Node *Parser::expectPrimaryExpression(TokIterator &i) {
	switch (i->mType) {
		case Token::LeftParenthese: {
			i++;
			ast::Node *expr = expectExpression(i);
			if (i->mType != Token::RightParenthese) {
				emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting right parenthese, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
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
				emit error(ErrorCodes::ecCantParseInteger, tr("Cannot parse integer \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer;
			intN->mValue = val;
			i++;
			return intN;
		}
		case Token::IntegerHex: {
			bool success;
			int val = i->toString().toInt(&success,16);
			if (!success) {
				emit error(ErrorCodes::ecCantParseInteger, tr("Cannot parse integer \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			ast::Integer *intN = new ast::Integer;
			intN->mValue = val;
			i++;
			return intN;
		}
		case Token::Float: {
			bool success;
			float val = i->toString().toFloat(&success);
			if (!success) {
				emit error(ErrorCodes::ecCantParseFloat, tr("Cannot parse float \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			ast::Float *floatN = new ast::Float;
			floatN->mValue = val;
			i++;
			return floatN;
		}
		case Token::String: {
			ast::String *str = new ast::String;
			str->mValue = i->toString();
			i++;
			return str;
		}
		case Token::Identifier: { //variable, type pointer's field, function call or array subscript
			int line = i->mLine;
			QFile *file = i->mFile;
			QString name = i->toString();
			i++;
			if (i->mType == Token::LeftParenthese) { //Function call or array subscript
				i++;
				ast::FunctionCallOrArraySubscript *ret = new ast::FunctionCallOrArraySubscript;
				ret->mFile = file;
				ret->mLine = line;
				ret->mName = name;
				if (i->mType == Token::RightParenthese) {  //No params
					i++;
					return ret;
				}
				ast::Node *first = expectExpression(i);
				if (mStatus == Error) return 0;
				ret->mParams.append(first);
				while (i->mType == Token::Comma) { // ,
					i++;
					ast::Node *rest = expectExpression(i);
					if (mStatus == Error) return 0;
					ret->mParams.append(rest);
				}
				if (i->mType != Token::RightParenthese) {
					emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting right parenthese, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
					mStatus = Error;
					return 0;
				}
				i++;
				return ret;
			}

			if (i->mType == Token::opTypePtrField) {// typeptr\field
				i++;
				if (i->mType != Token::Identifier) {
					emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting name of field after '\\', got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
					return 0;
				}
				QString fieldName = i->toString();
				i++;
				ast::Variable::VarType fieldTy = tryVarType(i);
				if (mStatus == Error) return 0;
				ast::TypePtrField *ret = new ast::TypePtrField;
				ret->mTypePtrVar = name;
				ret->mFieldName = fieldName;
				ret->mFieldType = fieldTy;
				return ret;
			}
			ast::Variable::VarType varTy = tryVarType(i);
			if (mStatus == Error) return 0;
			ast::Variable *var = new ast::Variable;
			var->mName = name;
			var->mVarType = varTy;
			return var;
		}
			//String & Float conversion functions
		case Token::kFloat:
		case Token::kString: {
			QString name = i->toString();
			i++;
			if (i->mType == Token::LeftParenthese) { //Function call or array subscript
				i++;
				ast::Node *first = expectExpression(i);
				if (mStatus == Error) return 0;
				if (i->mType != Token::RightParenthese) {
					emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting right parenthese, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
					mStatus = Error;
					return 0;
				}
				ast::FunctionCallOrArraySubscript *ret = new ast::FunctionCallOrArraySubscript;
				ret->mName = name;
				ret->mParams.append(first);
				i++;
				return ret;
			}
			else {
				emit error(ErrorCodes::ecExpectingLeftParenthese, tr("Expecting left parenthese, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
		}
		case Token::kNew: {
			i++;
			if (i->mType != Token::LeftParenthese) {
				emit error(ErrorCodes::ecExpectingLeftParenthese, tr("Expecting left parenthese after ´\"New\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;
			if (i->mType != Token::Identifier) {
				emit error(ErrorCodes::ecExpectingLeftParenthese, tr("Expecting name of a type, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			QString typeName = i->toString();
			i++;
			if (i->mType != Token::RightParenthese) {
				emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting right parenthese, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;
			ast::New *n = new ast::New;
			n->mTypeName = typeName;
			return n;
		}
	}
	Token tok = *i;
	emit error(ErrorCodes::ecExpectingPrimaryExpression, tr("Expecting primary expression, got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
	mStatus = Error;
	return 0;
}


ast::Node *Parser::tryAssignmentExpression(TokIterator &i) {
	if (i->mType != Token::Identifier) {
		return 0;
	}
	QString name = i->toString();
	TokIterator begin = i;
	ast::Node *var = 0;
	int line = i->mLine;
	QFile *file = i->mFile;
	i++;
	if (i->mType == Token::opTypePtrType) {
		i++;
		if (i->mType != Token::Identifier) {
			emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting the name of a type after '.', got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		ast::Variable *ret = new ast::Variable;
		ret->mName = name;
		ret->mVarType = ast::Variable::TypePtr;
		ret->mTypeName = i->toString();
		i++;
		var = ret;
	}
	else if (i->mType == Token::opTypePtrField) {
		i++;
		if (i->mType != Token::Identifier) {
			emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting the name of a field after '\\', got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
			mStatus = Error;
			return 0;
		}
		ast::TypePtrField *ret = new ast::TypePtrField;
		ret->mTypePtrVar = name;
		ret->mFieldName = i->toString();
		i++;
		ret->mFieldType = tryVarType(i);
		if (mStatus == Error) return 0;
		var = ret;
	}
	else {
		ast::Variable::VarType type = tryVarType(i);
		if (mStatus == Error) return 0;
		ast::Variable *ret = new ast::Variable;
		ret->mName = name;
		ret->mVarType = type;
		var = ret;
	}


	if (i->mType == Token::opEqual) {
		i++;
		ast::Node *expr = expectExpression(i);
		if (mStatus == Error) return 0;
		ast::AssignmentExpression *ret = new ast::AssignmentExpression;
		ret->mVariable = var;
		ret->mExpression = expr;
		ret->mFile = file;
		ret->mLine = line;
		return ret;
	}

	i = begin; //Reset iterator
	return 0;
}

ast::FunctionParametreDefinition Parser::expectFunctionParametreDefinition(Parser::TokIterator &i) {
	ast::FunctionParametreDefinition ret;
	expectVariableOrTypePtrDefinition(&ret.mVariable, i);
	if (mStatus == Error) return ret;

	if (i->mType == Token::opEqual) {
		i++;
		ret.mDefaultValue = expectExpression(i);
		if (mStatus == Error) return ret;
	}
	ret.mDefaultValue = 0;
	return ret;
}


ast::Node *Parser::tryRepeatStatement(Parser::TokIterator &i) {
	if (i->mType == Token::kRepeat) {
		QFile *file = i->mFile;
		int startLine = i->mLine;
		i++;
		expectEndOfStatement(i);
		if (mStatus == Error) return 0;
		ast::Block block = expectBlock(i);
		if (mStatus == Error) return 0;
		if (i->mType == Token::kUntil) {
			int endLine = i->mLine;
			i++;
			ast::Node *condition = expectExpression(i);
			if (mStatus == Error) return 0;
			ast::RepeatUntilStatement *ret = new ast::RepeatUntilStatement;
			ret->mFile = file;
			ret->mStartLine = startLine;
			ret->mEndLine = endLine;
			ret->mBlock = block;
			ret->mCondition = condition;
			return ret;
		}
		if (i->mType == Token::kForever) {
			ast::RepeatForeverStatement *ret = new ast::RepeatForeverStatement;
			ret->mBlock = block;
			ret->mFile = file;
			ret->mStartLine = startLine;
			ret->mEndLine = i->mLine;
			i++;
			return ret;
		}
		emit error(ErrorCodes::ecExpectingEndOfRepeat, tr("Expecting \"Until\" or \"Forever\", got \"%1\""), i->mLine, i->mFile);
		mStatus = Error;
		return 0;
	}

	return 0;
}

ast::Node *Parser::tryForStatement(Parser::TokIterator &i) {
	//TODO: Better error reporting
	if (i->mType == Token::kFor) {
		QFile *file = i->mFile;
		int startLine = i->mLine;
		i++;
		QString varName = expectIdentifier(i);
		if (mStatus == Error) return 0;

		//For-Each
		if (i->mType == Token::opTypePtrType) {
			i++;
			QString typeName1 = expectIdentifier(i);
			if (mStatus == Error) return 0;

			if (i->mType != Token::opEqual) {
				if (i->mType == Token::kEach) {
					emit error(ErrorCodes::ecExpectingAssignment, tr("Expecting assignment before \"To\""), i->mLine, i->mFile);
					mStatus = ErrorButContinue;
				}
				else {
					emit error(ErrorCodes::ecExpectingAssignment, tr("Expecting assignment after \"For\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
					mStatus = Error;
					return 0;
				}
			}
			else {
				i++;
			}
			if (i->mType != Token::kEach) {
				if (i->mType == Token::kTo) {
					emit error(ErrorCodes::ecExpectingVariable, tr("Use type pointer in For-Each loop"), i->mLine, i->mFile);
					mStatus = Error;
					return 0;
				}
				emit error(ErrorCodes::ecExpectingEach, tr("Expecting \"Each\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;

			QString typeName2 = expectIdentifier(i);

			if (typeName1 != typeName2) {
				emit error(ErrorCodes::ecTypesDontMatch, tr("The type \"%1\" of the type pointer doesn't match the type name \"%2\" after Each").arg(typeName1, typeName2), i->mLine, i->mFile);
				mStatus = ErrorButContinue;
			}
			expectEndOfStatement(i);
			ast::Block block = expectBlock(i);

			if (i->mType != Token::kNext) {
				emit error(ErrorCodes::ecExpectingNext, tr("Expecting \"Next\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			int endLine = i->mLine;
			i++;

			QString varName2 = expectIdentifier(i);
			if (varName != varName2) {
				emit error(ErrorCodes::ecForAndNextDontMatch, tr("Variable name \"%1\" doesn't match variable name \"%2\" at line %3").arg(varName2, varName2, QString::number(startLine)), i->mLine, i->mFile);
				mStatus = ErrorButContinue;
			}

			ast::ForEachStatement *ret = new ast::ForEachStatement;
			ret->mFile = file;
			ret->mStartLine = startLine;
			ret->mEndLine = endLine;
			ret->mBlock = block;
			ret->mVarName = varName;
			ret->mTypeName = typeName1;
			return ret;
		}


		{ //For-To
			ast::Variable::VarType varType = tryVarType(i);
			if (mStatus == Error) return 0;
			if (i->mType != Token::opEqual) {
				if (i->mType == Token::kTo) {
					emit error(ErrorCodes::ecExpectingAssignment, tr("Expecting assignment before \"To\""), i->mLine, i->mFile);
					mStatus = ErrorButContinue;
				}
				else {
					emit error(ErrorCodes::ecExpectingAssignment, tr("Expecting assignment after \"For\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
					mStatus = Error;
					return 0;
				}
			}
			else {
				i++;
			}


			ast::Node *from = expectExpression(i);
			if (mStatus == Error) return 0;

			if (i->mType != Token::kTo) {
				if (i->mType == Token::kEach) {
					emit error(ErrorCodes::ecExpectingTypePtr, tr("Expecting type pointer before \"Each\""), i->mLine, i->mFile);
					mStatus = Error;
					return 0;
				}
				emit error(ErrorCodes::ecExpectingTo, tr("Expecting \"To\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;

			ast::Node *to = expectExpression(i);
			if (mStatus == Error) return 0;

			ast::Node *step = 0;
			if (i->mType == Token::kStep) {
				i++;
				step = expectExpression(i);
				if (mStatus == Error) return 0;
			}

			expectEndOfStatement(i);
			if (mStatus == Error) return 0;
			ast::Block block = expectBlock(i);
			if (mStatus == Error) return 0;

			if (i->mType != Token::kNext) {
				emit error(ErrorCodes::ecExpectingNext, tr("Expecting \"Next\", got \"%1\"").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			int endLine = i->mLine;
			i++;
			QString varName2 = expectIdentifier(i);
			ast::Variable::VarType varType2 = tryVarType(i);
			if (mStatus == Error) return 0;
			if (varName != varName2) {
				emit error(ErrorCodes::ecForAndNextDontMatch, tr("Variable name \"%1\" doesn't match variable name \"%2\" at line %3").arg(varName2, varName2, QString::number(startLine)), i->mLine, i->mFile);
				mStatus = ErrorButContinue;
			}
			else if (varType != varType2) {
				emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Variable already defined with another type"), endLine, file);
				mStatus = ErrorButContinue;
			}



			ast::ForToStatement *forTo = new ast::ForToStatement;
			forTo->mBlock = block;
			forTo->mVarName = varName;
			forTo->mVarType = varType;
			forTo->mFile = file;
			forTo->mStartLine = startLine;
			forTo->mEndLine = endLine;
			forTo->mFrom = from;
			forTo->mTo = to;
			forTo->mStep = step;
			return forTo;
		}
	}

	return 0;
}

ast::Node *Parser::tryFunctionOrCommandCallOrArraySubscriptAssignment(Parser::TokIterator &i) {
	switch(i->mType) {
		case Token::kExit: {
			ast::Exit *ret = new ast::Exit;
			ret->mLine = i->mLine;
			ret->mFile = i->mFile;
			i++;
			return ret;
		}
		case Token::kEnd: {
			ast::CommandCall *ret = new ast::CommandCall;
			ret->mLine = i->mLine;
			ret->mFile = i->mFile;
			ret->mName = i->toString();
			i++;
			return ret;
		}
		case Token::Identifier: {
				QFile *file = i->mFile;
				int line = i->mLine;
				QString name = i->toString();
				i++;
				bool maybeFunction = false;
				if (i->mType == Token::LeftParenthese) {
					maybeFunction = true;
					i++;
					if (i->mType == Token::RightParenthese) { //Function without parameters
						ast::FunctionCallOrArraySubscript *ret = new ast::FunctionCallOrArraySubscript;
						ret->mFile = file;
						ret->mLine = line;
						ret->mName = name;
						return ret;
					}
				}
				else {
					if (i->isEndOfStatement()) { //Command without parameters
						ast::CommandCall *ret = new ast::CommandCall;
						ret->mFile = file;
						ret->mLine = line;
						ret->mName = name;
						return ret;
					}
				}
				QList<ast::Node*> parameters;
				parameters.append(expectExpression(i));
				if (mStatus == Error) return 0;
				while (i->mType == Token::Comma) {
					i++;
					parameters.append(expectExpression(i));
					if (mStatus == Error) return 0;
				}
				if (i->mType == Token::RightParenthese) {
					i++;
					if (!maybeFunction) {
						emit error(ErrorCodes::ecExpectingEndOfStatement, tr("Unexpected token \"%1\". Expecting the end of the statement.").arg(i->toString()), line, file);
						mStatus = Error;
						return 0;
					}
					if (i->mType == Token::opEqual) { // adc ( x + 2) = ****
						i++;
						ast::Node *val = expectExpression(i);
						if (mStatus == Error) return 0;
						if (parameters.size() == 1) {
							if (i->mType == Token::Comma) {// abc (x + 2) = 31, ***
								ast::Node *f = parameters.first();
								parameters.clear();

								//Combine (x + 2) = 31
								ast::Expression *firstParam = new ast::Expression;
								firstParam->mFirst = f;
								firstParam->mRest = QList<ast::Operation>() << ast::Operation(ast::opEqual, val);

								parameters.append(firstParam);
								while (i->mType == Token::Comma) {
									i++;
									parameters.append(expectExpression(i));
									if (mStatus == Error) return 0;
								}
								ast::CommandCall *ret = new ast::CommandCall;
								ret->mParams = parameters;
								ret->mFile = file;
								ret->mLine = line;
								ret->mName = name;
							}
							else {
								ast::CommandCallOrArraySubscriptAssignmentExpression *ret = new ast::CommandCallOrArraySubscriptAssignmentExpression;
								ret->mIndexOrExpressionInParentheses = parameters.first();
								ret->mEqualTo = val;
								ret->mName = name;
								ret->mLine = line;
								ret->mFile = file;
								return ret;
							}
						}
						else {
							ast::ArraySubscriptAssignmentExpression *ret = new ast::ArraySubscriptAssignmentExpression;
							ret->mArrayName = name;
							ret->mValue = val;
							ret->mSubscripts = parameters;
							ret->mFile = file;
							ret->mLine = line;
							return ret;
						}
					}
					ast::FunctionCallOrArraySubscript *ret = new ast::FunctionCallOrArraySubscript;
					ret->mName = name;
					ret->mFile = file;
					ret->mLine = line;
					ret->mParams = parameters;
					return ret;
				}
				else {
					if (maybeFunction) {
						emit error(ErrorCodes::ecExpectingRightParenthese, tr("Expecting ')', got '%1'").arg(i->toString()), line, file);
						mStatus = Error;
						return 0;
					}
					ast::CommandCall *ret = new ast::CommandCall;
					ret->mFile = file;
					ret->mLine = line;
					ret->mName = name;
					ret->mParams = parameters;
					return ret;
				}



		}
	}
	return 0;
}
