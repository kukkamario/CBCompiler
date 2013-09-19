#ifndef SYMBOL_H
#define SYMBOL_H
#include "global.h"
#include <QString>
class QFile;
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
			stArray,
			stType,
			stLabel,
			stDefaultValueType,
			stCustomValueType
		};

		Symbol(const QString &name,const QString &f, int line);
		virtual Type type() const = 0;
		virtual QString info() const = 0; //Compiler debugging information
		QString name() const {return mName;}
		QString file() const {return mFile;}
		int line() const { return mLine;}
		virtual bool isValueTypeSymbol() const { return false; }
	protected:
		QString mName;
		QString mFile;
		int mLine;
};

#endif // SYMBOL_H
