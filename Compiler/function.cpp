#include "function.h"

Function::Function(const std::string &name, const CodePoint &cp):
	mName(name), mReturnValue(0), mFunction(0),mCodePoint(cp), mRequiredParams(0) {
}
