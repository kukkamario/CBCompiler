#include "token.h"
#include <QtAlgorithms>
#include <iostream>
#include <QFile>
#include <QDebug>

static const char *const tokenNames[] = {
	"EndOfTokens",
	"EOL",
	"Integer",
	"IntegerHex",
	"Float",
	"String",
	"Identifier",
	"Label",
	"Colon", // :
	"Comma",

	"IntegerTypeMark", //%
	"StringTypeMark", //$
	"FloatTypeMark", //#

	"LeftParenthese",
	"RightParenthese",

	"LeftSquareBracket",
	"RightSquareBracket",
	//Operators

	"OperatorsBegin",
	"opAssign",
	"opEqual",
	"opNotEqual",
	"opGreater",
	"opLess",
	"opGreaterEqual",
	"opLessEqual",
	"opPlus",
	"opMinus",
	"opMultiply",
	"opPower",
	"opMod",
	"opShl",
	"opShr",
	"opSar",
	"opDivide",
	"opAnd",
	"opOr",
	"opXor",
	"opNot",
	"opDot",  //   Dot
	"OperatorsEnd",

	//Keywords
	"KeywordsBegin",
	"kIf",
	"kThen",
	"kElseIf",
	"kElse",
	"kEndIf",
	"kSelect",
	"kCase",
	"kDefault",
	"kEndSelect",
	"kFunction",
	"kReturn",
	"kEndFunction",
	"kType",
	"kStruct",
	"kField",
	"kEndType",
	"kEndStruct",
	"kWhile",
	"kWend",
	"kRepeat",
	"kForever",
	"kUntil",
	"kGoto",
	"kGosub",
	"kFor",
	"kTo",
	"kEach",
	"kStep",
	"kNext",
	"kDim",
	"kRedim",
	"kClearArray",
	"kConst",
	"kGlobal",
	"kData",
	"kRead",
	"kRestore",
	"kAs",
	"kInclude",
	"kExit",
	"kEnd",

	"kNew",
	"kFirst",
	"kLast",
	"kBefore",
	"kAfter",

	"KeywordsEnd",
	"TypeCount"
};
const char *getTokenName(Token::Type t) {
	if (t < 0 || t >= Token::TypeCount) {
		return "Invalid token type";
	}
	return tokenNames[t];
}

QString Token::toString() const {
	/*QString s;
	s.resize(reinterpret_cast<int>(mEnd - mBegin) / sizeof(QChar));
	memcpy(s.data(), mBegin, reinterpret_cast<int>(mEnd - mBegin));
	return s;*/

	//Tokens must not be deleted. Otherwise bad things happen.
	if (mBegin >= mEnd) return QString();
	return QString::fromRawData(mBegin, mEnd - mBegin);
}

QString Token::typeToString() const {
	return getTokenName(mType);
}



QString Token::info() const {
	QString ret(mCodePoint.toString());
	ret += " ";
	ret += getTokenName(mType);
	ret += "  \"";
	ret += toString();
	ret += '"';
	return ret;
}

void Token::print() const
{
	qDebug() << info();
}

bool Token::isKeyword() const {
	return KeywordsBegin < mType && mType < KeywordsEnd;
}

bool Token::isEndOfStatement() const {
	return mType == EOL || mType == Colon;
}
bool Token::isOperator() const {
	return OperatorsBegin < mType && mType < OperatorsEnd;
}
