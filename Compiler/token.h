#ifndef TOKEN_H
#define TOKEN_H
#include <QString>
#include "codepoint.h"

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
			opAssign,
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
			kClass,
			kField,
			kEndType,
			kEndClass,
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

			KeywordsEnd,
			TypeCount
		};

		Token(Type t, QString::ConstIterator begin, QString::ConstIterator end, const CodePoint &cp) : mType(t), mBegin(begin), mEnd(end), mCodePoint(cp) {}
		~Token() { }
		Type type() const { return mType; }
		const CodePoint &codePoint() const { return mCodePoint; }
		QString file() const { return mCodePoint.file(); }
		int line() const { return mCodePoint.line(); }
		int column() const { return mCodePoint.column(); }

		QString toString() const;
		QString typeToString() const;
		QString info() const;
		void print() const;
		bool isOperator() const;
		bool isKeyword() const;
		bool isEndOfStatement() const;

		QString::ConstIterator begin() const { return mBegin; }
		QString::ConstIterator end() const { return mEnd; }
	private:
		Type mType;
		QString::ConstIterator mBegin;
		QString::ConstIterator mEnd;
		CodePoint mCodePoint;
};

#endif // TOKEN_H
