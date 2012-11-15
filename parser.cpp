#include "parser.h"
#include "errorhandler.h"
Parser::Parser():
	Status(Ok)
{
}

ast::Program *Parser::parse(const QList<Token> &tokens)
{
	ast::Program *program = new ast::Program;
	for (TokIterator i = tokens.begin(); i != tokens.end(); i++) {

	}
	return program;
}

ast::GlobalDefinition *Parser::tryGlobalDefinition(Parser::TokIterator &i) {
	if (i->mType == Token::kGlobal) {
		i++;
		ast::Variable *var = expectVariableOrTypePtrDefinition(i);
		if (mStatus == Error) return 0;
		ast::GlobalDefinition *ret = new ast::GlobalDefinition;
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
				emit error(ErrorHandler::ecExpectingIdentifier, tr("Expecting variable type after \"as\", got %1").arg(i->toString()), i->mLine, i->mFile);
				return ast::Variable::Invalid;
		}
	}
	return ast::Variable::Default;
}

ast::VariableDefinition *Parser::tryVariableDefinition(Parser::TokIterator &i) {
	Parser::TokIterator begin = i;
	if (i->mType == Token::kDim) {
		i++;
		ast::Variable *var = expectVariableOrTypePtrDefinition(i);
		if (mStatus == Error) return 0;
		if (i->mType == Token::LeftParenthese) { //Array definition
			i = begin;
		}
		ast::VariableDefinition *ret = new ast::VariableDefinition;
		ret->mDefinitions.append(var);
		while (i->mType == Token::Comma) {
			i++;
			ast::Variable *var = expectVariableOrTypePtrDefinition(i);
			if (mStatus == Error) return 0;
			ret->mDefinitions.append(var);
		}
		return ret;
	}
	return 0;
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
					emit error(ErrorHandler::ecExpectingIdentifier, tr("Expecting variable type after \"as\", got %1").arg(i->toString()), i->mLine, i->mFile);
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

QString Parser::expectIdentifier(Parser::TokIterator &i) {
	if (i->mType == Token::Identifier) {
		QString ret = i->toString();
		i++;
		return ret;
	}

	emit error(ErrorHandler::ecExpectingIdentifier, tr("Expecting identifier, got %1").arg(i->toString()), i->mLine, i->mFile);
	mStatus = Error;
	return QString();
}

void Parser::expectEndOfStatement(Parser::TokIterator &i) {
	if (i->isEndOfStatement()) {
		i++;
		return;
	}
	mStatus = Error;
	emit error(ErrorHandler::ecExpectingEndOfStatement, tr("Expecting end of line or ':', got %1").arg(i->toString()), i->mLine, i->mFile);
	i++;
}


ast::Variable *Parser::expectVariableOrTypePtrDefinition(Parser::TokIterator &i) {
	ast::Variable * var = tryVariableOrTypePtrDefinition(i);
	if (!var) {
		emit error(ErrorHandler::ecExpectingIdentifier, tr("Expecting variable, got %1").arg(i->toString()), i->mLine, i->mFile);
	}
	return var;
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
			emit error(ErrorHandler::ecExpectingIdentifier, tr("Expecting type name after '.', got %1").arg(i->toString()), i->mLine, i->mFile);
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
		i++;
		return ret;
	}
}

ast::Node *Parser::tryDim(Parser::TokIterator &i) {
	Parser::TokIterator begin = i;
	if (i->mType == Token::kDim) {
		i++;
		if (i->mType != Token::Identifier) {
			emit error(ErrorHandler::ecExpectingIdentifier, tr("Expecting variable name after \"Dim\", got %1").arg(i->toString()), i->mLine, i->mFile);
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
				emit error(ErrorHandler::ecExpectingRightParenthese, tr("Expecting right parenthese, got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;

			if (varType == ast::Variable::Default) {
				varType = tryVarAsType(i);
				if (mStatus == Error) return 0;
			}
			ast::ArrayDefinition *arr = new ast::ArrayDefinition;
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
							emit error(ErrorHandler::ecExpectingIdentifier, tr("Expecting type name after '.', got %1").arg(i->toString()), i->mLine, i->mFile);
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

ast::IfStatement *Parser::tryIfStatement(Parser::TokIterator &i) {
	if (i->mType == Token::kIf) {
		i++;
		ast::Node *condition = expectExpression(i);
		if (mStatus == Error) return 0;
		if (i->mType == Token::kThen) {
			i++;
			if (!i->isEndOfStatement()) { //Inline if
				i++;

			}
		}
		else {
			if (!i->isEndOfStatement()) {
				emit error(ErrorHandler::ecExpectingEndOfStatement, tr("Expecting end of line or ':', got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
		}
	}

	return 0;
}

ast::Block Parser::expectBlock(Parser::TokIterator &i) {
	ast::Block block;
	while (true) {
		ast::Node * n = tryDim(i);
		if (mStatus == Error) return block;
		if (n) {
			block.append(n);
			if (!i->isEndOfStatement()) {
				if (i->mType == Token::EndOfTokens) return block;
				emit error(ErrorHandler::ecExpectingEndOfStatement, tr("Expecting end of line or ':', got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return block;
			}
			i++;
			continue;
		}
		n = tryGlobalDefinition(i);
		if (mStatus == Error) return block;
		if (n) {
			block.append(n);
			if (!i->isEndOfStatement()) {
				if (i->mType == Token::EndOfTokens) return block;
				emit error(ErrorHandler::ecExpectingEndOfStatement, tr("Expecting end of line or ':', got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return block;
			}
			i++;
			continue;
		}

		n = tryIfStatement(i);
		if (mStatus == Error) return block;
		if (n) {
			block.append(n);
			if (!i->isEndOfStatement()) {
				if (i->mType == Token::EndOfTokens) return block;
				emit error(ErrorHandler::ecExpectingEndOfStatement, tr("Expecting end of line or ':', got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return block;
			}
			i++;
			continue;
		}
	}
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
				emit error(ErrorHandler::ecExpectingRightParenthese, tr("Expecting right parenthese, got %1").arg(i->toString()), i->mLine, i->mFile);
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
				emit error(ErrorHandler::ecCannotParseInteger, tr("Cannot parse integer \"%1\"").arg(i->toString()), i->mLine, i->mFile);
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
				emit error(ErrorHandler::ecCannotParseInteger, tr("Cannot parse integer \"%1\"").arg(i->toString()), i->mLine, i->mFile);
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
				emit error(ErrorHandler::ecCannotParseFloat, tr("Cannot parse float \"%1\"").arg(i->toString()), i->mLine, i->mFile);
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
			QString name = i->toString();
			i++;
			if (i->mType == Token::LeftParenthese) { //Function call or array subscript
				i++;
				ast::FunctionCallOrArraySubscript *ret = new ast::FunctionCallOrArraySubscript;
				ret->mName = name;
				if (i->mType == Token::RightParenthese) {  //No params
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
					emit error(ErrorHandler::ecExpectingRightParenthese, tr("Expecting right parenthese, got %1").arg(i->toString()), i->mLine, i->mFile);
					mStatus = Error;
					return 0;
				}
				i++;
				return ret;
			}

			if (i->mType == Token::opTypePtrField) {// typeptr\field
				i++;
				if (i->mType != Token::Identifier) {
					emit error(ErrorHandler::ecExpectingIdentifier, tr("Expecting name of field after '\\', got %1").arg(i->toString()), i->mLine, i->mFile);
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
					emit error(ErrorHandler::ecExpectingRightParenthese, tr("Expecting right parenthese, got %1").arg(i->toString()), i->mLine, i->mFile);
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
				emit error(ErrorHandler::ecExpectingLeftParenthese, tr("Expecting left parenthese, got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
		}
		case Token::kNew: {
			i++;
			if (i->mType != Token::LeftParenthese) {
				emit error(ErrorHandler::ecExpectingLeftParenthese, tr("Expecting left parenthese after ´\"New\", got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;
			if (i->mType != Token::Identifier) {
				emit error(ErrorHandler::ecExpectingLeftParenthese, tr("Expecting name of a type, got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			QString typeName = i->toString();

			if (i->mType != Token::RightParenthese) {
				emit error(ErrorHandler::ecExpectingRightParenthese, tr("Expecting right parenthese, got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;
			ast::New *n = new ast::New;
			n->mTypeName = typeName;
			return n;
		}
	}
	emit error(ErrorHandler::ecExpectingPrimaryExpression, tr("Expecting primary expression, got %1").arg(i->toString()), i->mLine, i->mFile);
	mStatus = Error;
	return 0;
}


ast::AssignmentExpression *Parser::tryAssignmentExpression(TokIterar &i) {
	ast::Variable *var = tryVariableOrTypePtrDefinition(i);
	if (!var) return 0;
	if (i->mType == Token::opEqual) {
		i++;
		ast::Node *expr = expectExpression(i);
		if (mStatus == Error) return 0;
		ast::AssignmentExpression *ret = new ast::AssignmentExpression;
		ret->mVariable = var;
		ret->mExpression = expr;
		return ret;
	}
	return 0;
}


ast::Node *Parser::tryRepeatStatement(Parser::TokIterator &i) {
	if (i->mType == Token::kRepeat) {
		i++;
		expectEndOfStatement(i);
		if (mStatus == Error) return 0;
		ast::Block block = expectBlock(i);
		if (mStatus == Error) return 0;
		if (i->mType == Token::kUntil) {
			i++;
			ast::Node *condition = expectExpression(i);
			if (mStatus == Error) return 0;
			ast::RepeatUntilStatement *ret = new ast::RepeatUntilStatement;
			ret->mBlock = block;
			ret->mCondition = condition;
			return ret;
		}
		if (i->mType == Token::kForever) {
			i++;
			ast::RepeatForeverStatement *ret = new ast::RepeatForeverStatement;
			ret->mBlock = block;
			return ret;
		}
		emit error(ErrorHandler::ecExpectingEndOfRepeat, tr("Expecting \"Until\" or \"Forever\", got %1"), i->mLine, i->mFile);
		mStatus = Error;
		return 0;
	}

	return 0;
}

ast::Node *Parser::tryForStatement(Parser::TokIterator &i) {
	//TODO: Better error reporting
	if (i->mType == Token::kFor) {
		int forLine = i->mLine;
		i++;
		QString varName = expectIdentifier(i);
		if (mStatus == Error) return 0;

		//For-Each
		if (i->mType == Token::opTypePtrType) {

		}


		{ //For-To
			ast::Variable::VarType varType = tryVarType(i);
			if (mStatus == Error) return 0;
			if (i->mType != Token::opEqual) {
				if (i->mType == Token::kTo) {
					emit error(ErrorHandler::ecExpectingAssignment, tr("Expecting assignment before \"To\""), i->mLine, i->mFile);
					mStatus = ErrorButContinue;
				}
				else {
					emit error(ErrorHandler::ecExpectingAssignment, tr("Expecting assignment after \"For\", got %1").arg(i->toString()), i->mLine, i->mFile);
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
				emit error(ErrorHandler::ecExpectingTo, tr("Expecting \"To\", got %1").arg(i->toString()), i->mLine, i->mFile);
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
			expectBlock(i);
			if (mStatus == Error) return 0;

			if (i->mType != Token::kNext) {
				emit error(ErrorHandler::ecExpectingNext, tr("Expecting \"Next\", got %1").arg(i->toString()), i->mLine, i->mFile);
				mStatus = Error;
				return 0;
			}
			i++;
			QString varName2 = expectIdentifier(i);
			if (mStatus == Error) return 0;
			if (varName != varName2) {
				emit error(ErrorHandler::ecForAndNextDontMatch, tr("Variable name \"%1\" doesn't match variable name \"%2\" at line %3").arg(varName2, varName2, QString::number(forLine)), i->mLine, i->mFile);
				mStatus = ErrorButContinue;
			}
		}
	}

	return 0;
}
