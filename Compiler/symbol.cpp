#include "symbol.h"

Symbol::Symbol(const QString &name, const QString &f, int line):
	mName(name),
	mFile(f),
	mLine(line)
{
}
