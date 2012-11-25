#include "symbol.h"

Symbol::Symbol(const QString &name, QFile *f, int line):
	mName(name),
	mFile(f),
	mLine(line)
{
}
