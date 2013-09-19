#include "function.h"

Function::Function(const QString &name, const QString &f, int line):
	mName(name), mReturnValue(0), mFunction(0),mFile(f), mLine(line), mRequiredParams(0) {
}
