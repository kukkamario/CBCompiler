#ifndef TOKEN_H
#define TOKEN_H
#include <QString>
class QFile;
struct Token
{
		enum Type {
			EndOfTokens,
			EOL,
			Integer,
			IntegerHex,
			Float,
			FloatLeadingDot,
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
			opTypePtrField,
			opTypePtrType,  //   Dot
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
			kField,
			kEndType,
			kNew,
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
			kEnd,
			kInteger,
			kFloat,
			kString,
			kShort,
			kByte,
			kAs,
			kInclude,
			kExit,
			KeywordsEnd,
			TypeCount
		};

		Token(Type t, QString::ConstIterator begin, QString::ConstIterator end, int line, QFile *f) : mType(t), mBegin(begin), mEnd(end), mLine(line), mFile(f) {}
		QString toString() const;
		QString typeToString() const;
		QString info() const;
		void print() const;
		bool isOperator() const;
		bool isKeyword() const;
		bool isEndOfStatement() const;

		Type mType;
		QString::ConstIterator mBegin;
		QString::ConstIterator mEnd;
		int mLine;
		QFile *mFile;
};

#endif // TOKEN_H
