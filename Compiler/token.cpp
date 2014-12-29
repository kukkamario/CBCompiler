#include "token.h"
#include <iostream>

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
	"kDelete",
	"kArraySize"

	"KeywordsEnd",
	"TypeCount"
};
const char *getTokenName(Token::Type t) {
	if (t < 0 || t >= Token::TypeCount) {
		return "Invalid token type";
	}
	return tokenNames[t];
}

const std::string &Token::toString() const {
	return mText;
}

boost::string_ref Token::typeToString() const {
	return getTokenName(mType);
}



std::string Token::info() const {
	std::string ret(mCodePoint.toString());
	ret += " ";
	ret += getTokenName(mType);
	ret += "  \"";
	ret += toString();
	ret += '"';
	return ret;
}

void Token::print() const {
	std::cout << info();
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
