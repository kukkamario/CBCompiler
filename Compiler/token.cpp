#include "token.h"
#include <QtAlgorithms>
#include <iostream>
#include <QFile>
#include <QDebug>
static char *tokenNames[] = {
	"EndOfTokens",
	"EOL",
	"Integer",
	"IntegerHex",
	"Float",
	"FloatLeadingDot",
	"String",
	"Identifier",
	"Label",
	"Colon", // :
	"Comma",

	"IntTypeMark", //%
	"StringTypeMark", //$
	"FloatTypeMark", //#

	"LeftParenthese",
	"RightParenthese",

	//Operators
	"OperatorsBegin",
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
	"opTypePtrField",
	"opTypePtrType",  //   Dot
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
	"kField",
	"kEndType",
	"kNew",
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
	"kEnd",
	"kInteger",
	"kFloat",
	"kString",
	"kShort",
	"kByte",
	"kAs",
	"kInclude",
	"kExit",
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



QString Token::info() const
{
	QString ret(mFile->fileName());
	ret += " [";
	ret += QString::number(mLine);
	ret += "]  ";
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
