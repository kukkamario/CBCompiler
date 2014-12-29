#ifndef TOKEN_H
#define TOKEN_H
#include <string>
#include "codepoint.h"
#include "utf8.h"
#include <boost/utility/string_ref.hpp>

class Token {
	public:
		enum Type {
			EndOfTokens = 0,
			EOL,
			Integer,
			IntegerHex,
			Float,
			String,
			Identifier,
			Label,
			Colon, // :
			Comma,

			IntegerTypeMark, //%
			StringTypeMark, //$
			FloatTypeMark, //#

			LeftParenthese,
			RightParenthese,

			LeftSquareBracket,
			RightSquareBracket,
			//Operators

			OperatorsBegin,
			opEqual,
			opNotEqual,
			opGreater,
			opLess,
			opGreaterEqual,
			opLessEqual,
			opPlus,
			opMinus,
			opMultiply,
			opPower,
			opMod,
			opShl,
			opShr,
			opSar,
			opDivide,
			opAnd,
			opOr,
			opXor,
			opNot,
			opDot,  //   Dot
			OperatorsEnd,

			//Keywords
			KeywordsBegin,
			kIf,
			kThen,
			kElseIf,
			kElse,
			kEndIf,
			kSelect,
			kCase,
			kDefault,
			kEndSelect,
			kFunction,
			kReturn,
			kEndFunction,
			kType,
			kStruct,
			kField,
			kEndType,
			kEndStruct,
			kWhile,
			kWend,
			kRepeat,
			kForever,
			kUntil,
			kGoto,
			kGosub,
			kFor,
			kTo,
			kEach,
			kStep,
			kNext,
			kDim,
			kRedim,
			kClearArray,
			kConst,
			kGlobal,
			kData,
			kRead,
			kRestore,
			kAs,
			kInclude,
			kExit,
			kEnd,

			kNew,
			kFirst,
			kLast,
			kBefore,
			kAfter,
			kDelete,
			kArraySize,

			KeywordsEnd,
			TypeCount
		};

		Token(Type t, const std::string &text, const CodePoint &cp) : mType(t), mText(text), mCodePoint(cp) {}
		~Token() { }
		Type type() const { return mType; }
		const CodePoint &codePoint() const { return mCodePoint; }
		boost::string_ref file() const { return mCodePoint.file(); }
		int line() const { return mCodePoint.line(); }
		int column() const { return mCodePoint.column(); }

		const std::string &toString() const;
		boost::string_ref typeToString() const;
		std::string info() const;
		void print() const;
		bool isOperator() const;
		bool isKeyword() const;
		bool isEndOfStatement() const;
	private:
		Type mType;
		std::string mText;
		CodePoint mCodePoint;
};

#endif // TOKEN_H
