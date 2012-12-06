#ifndef RUNTIMEFUNCTION_H
#define RUNTIMEFUNCTION_H
#include "function.h"
#include "llvm.h"
class Runtime;
class RuntimeFunction : public Function {
	public:
		RuntimeFunction(Runtime * r);
		bool construct(llvm::Function *func, const QString &name);
		bool isRuntimeFunction() const {return true;}
	private:
		Runtime *mRuntime;
};

#endif // RUNTIMEFUNCTION_H
