#ifndef SYMBOL_H
#define SYMBOL_H
#include "global.h"
#include "codepoint.h"
#include <string>

class Symbol {
	public:
		enum OverloadSearchError {
			oseNoError,
			oseCannotFindAny,
			oseFoundMultipleOverloads
		};
		enum Type {
			stVariable,
			stFunctionOrCommand,
			stConstant,
			stType,
			stLabel,
			stValueType
		};

		Symbol(const std::string &name,const CodePoint &cp);
		virtual Type type() const = 0;
		virtual std::string info() const = 0; //Compiler debugging information
		std::string name() const { return mName; }
		boost::string_ref file() const { return mCodePoint.file(); }
		int line() const { return mCodePoint.line(); }
		int column() const { return mCodePoint.column(); }
		const CodePoint &codePoint() const { return mCodePoint; }
		bool isRuntimeSymbol() const { return mCodePoint.isNull(); }

		virtual bool isValueTypeSymbol() const { return false; }
	protected:
		std::string mName;
		CodePoint mCodePoint;
};

#endif // SYMBOL_H
