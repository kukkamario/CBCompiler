#ifndef SYMBOL_H
#define SYMBOL_H
#include <QString>
class QFile;
class Symbol {
	public:
		enum Type {
			stVariable,
			stFunction,
			stCommand,
			stConstant,
			stType
		};

		Symbol(const QString &name, QFile *f, int line);
		virtual Type type() const = 0;
		virtual QString info() const = 0; //Compiler debugging information
		QString name()const {return mName;}
		QFile *file()const {return mFile;}
		int line()const{return mLine;}
	protected:
		QString mName;
		QFile *mFile;
		int mLine;
};

#endif // SYMBOL_H
