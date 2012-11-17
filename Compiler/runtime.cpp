#include "runtime.h"
#include <QDebug>

Runtime::Runtime():
	mModule(0),
	mCBMain(0)
{
}

bool Runtime::load(const QString &file) {

	llvm::SMDiagnostic diagnostic;
	mModule = llvm::ParseIRFile(file.toStdString(), diagnostic, llvm::getGlobalContext());

	if (!loadValueTypes()) return false;

	for (llvm::Module::FunctionListType::iterator i = mModule->getFunctionList().begin(); i != mModule->getFunctionList().end(); i++) {
		llvm::Function *func = &(*i);

		const char * const runtimeFuncNamePrefix = "CBF_";
		int c = 0;
		bool runtimeFunc = false;
		QString funcName;
		for (llvm::StringRef::const_iterator i = func->getName().begin(); i != func->getName().end(); i++) {
			if (runtimeFuncNamePrefix[c] != *i) {
				runtimeFunc = false;
				break;
			}

			c++;
			if (c == 4) {
				runtimeFunc = true;
				for (i = func->getName().begin(); i != func->getName().end(); i++) {
					funcName += *i;
				}
				break;
			}
		}
		if (!runtimeFunc || funcName.isEmpty()) continue;

		if (funcName == "CB_main") {
			mCBMain = func;
		}

		qDebug() << "Runtime function: " << funcName;

	}
	return true;
}


bool Runtime::loadValueTypes() {

	return true;
}
