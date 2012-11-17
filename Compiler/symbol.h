#ifndef SYMBOL_H
#define SYMBOL_H
#include <QString>

class Symbol {
	public:
		enum Type {
			stVariable,
			stFunction,
			stCommand,
			stType
		};

		Symbol(const QString &name);
		virtual Type type() const = 0;
		QString name()const {return mName;}
	protected:
		QString mName;
};

#endif // SYMBOL_H
