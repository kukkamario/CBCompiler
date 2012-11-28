#include "function.h"

Function::Function(QFile *f, int line):
	mReturnValue(0), mFunction(0),mFile(f), mLine(line) {
}
