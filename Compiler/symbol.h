#ifndef SYMBOL_H
#define SYMBOL_H
#include "global.h"
#include "codepoint.h"
#include <QString>

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

		Symbol(const QString &name,const CodePoint &cp);
		virtual Type type() const = 0;
		virtual QString info() const = 0; //Compiler debugging information
		QString name() const { return mName; }
		QString file() const { return mCodePoint.file(); }
		int line() const { return mCodePoint.line(); }
		int column() const { return mCodePoint.column(); }
		const CodePoint &codePoint() const { return mCodePoint; }
		bool isRuntimeSymbol() const { return mCodePoint.isNull(); }

		virtual bool isValueTypeSymbol() const { return false; }
	protected:
		QString mName;
		CodePoint mCodePoint;
};

#endif // SYMBOL_H
