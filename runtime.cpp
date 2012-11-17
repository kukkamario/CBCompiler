#include "runtime.h"

Runtime::Runtime()
{
}

bool Runtime::load(const QString &file) {

	llvm::SMDiagnostic diagnostic;
	mModule = llvm::ParseIRFile(file.toStdString(), diagnostic, llvm::getGlobalContext());
	llvm::Module::FunctionListType functionList = mModule->getFunctionList();

	for (llvm::Module::FunctionListType::const_iterator i = functionList.begin(); i != functionList.end(); i++) {
		const llvm::Function &func = *i;

		const char * const runtimeFuncNamePrefix = "CBF_";
		int c = 0;
		bool runtimeFunc = false;
		QString funcName;
		for (llvm::StringRef::const_iterator i = func.getName().begin(); i != func.getName().end(); i++) {
			if (runtimeFuncNamePrefix[c] != *i) {
				runtimeFunc = false;
				break;
			}

			c++;
			if (c == 4) {
				runtimeFunc = true;
				for (i = func.getName().begin(); i != func.getName().end(); i++) {
					funcName += *i;
				}
				break;
			}
		}
		if (!runtimeFunc || funcName.isEmpty()) continue;

		qDebug() << "Runtime function: " << funcName;

	}
}
