#include "runtime.h"
#include <QDebug>
#include "intvaluetype.h"
#include "stringvaluetype.h"
#include "shortvaluetype.h"
#include "floatvaluetype.h"
#include "bytevaluetype.h"
#include "booleanvaluetype.h"
#include "errorcodes.h"
static Runtime *runtimeInstance = 0;

Runtime::Runtime():
	mModule(0),
	mCBMain(0) {
	assert(runtimeInstance == 0);
	runtimeInstance = this;
}

Runtime::~Runtime() {
	runtimeInstance = 0;
}

bool Runtime::load(StringPool *strPool, const QString &file) {

	llvm::SMDiagnostic diagnostic;
	mModule = llvm::ParseIRFile(file.toStdString(), diagnostic, llvm::getGlobalContext());
	if (!mModule) return 0;

	if (!loadValueTypes(strPool)) return false;

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
		qDebug() << "Runtime function: " << funcName;

		addRuntimeFunction(func, funcName);
	}
	return true;
}


bool Runtime::loadValueTypes(StringPool *strPool) {
	//Int
	mIntValueType = new IntValueType(this, mModule);
	mValueTypes.append(mIntValueType);

	//String
	mStringValueType = new StringValueType(strPool, this, mModule);
	llvm::StructType *str = mModule->getTypeByName("struct.CB_StringData");
	if (!str) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Can't find \"struct.CB_StringData\" in runtime library bitcode"), 0, 0);
		return false;
	}
	mStringValueType->setStringType(str->getPointerTo());
	mValueTypes.append(mStringValueType);

	mFloatValueType = new FloatValueType(this, mModule);
	mValueTypes.append(mFloatValueType);

	mShortValueType = new ShortValueType(this, mModule);
	mValueTypes.append( mShortValueType);

	mByteValueType = new ByteValueType(this, mModule);
	mValueTypes.append(mByteValueType);

	mBooleanValueType = new BooleanValueType(this, mModule);
	mValueTypes.append(mBooleanValueType);
	return true;
}


void Runtime::addRuntimeFunction(llvm::Function *func, const QString &name) {
	if (name == "CB_main") {
		mCBMain = func;
		return;
	}
	if (name == "CB_StringConstruct") {
		if (!mStringValueType->setConstructFunction(func)) {
			emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CBF_CB_StringConstruct"), 0, 0);
		}
		return;
	}

	if (name == "CB_StringDestruct") {
		if (!mStringValueType->setDestructFunction(func)) {
			emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CBF_CB_StringDestruct"), 0, 0);
		}
		return;
	}

	if (name == "CB_StringAssign") {
		if (!mStringValueType->setAssignmentFunction(func)) {
			emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CBF_CB_StringAssign"), 0, 0);
		}
		return;
	}

	if (name == "CB_StringToInt") {
		if (!mStringValueType->setStringToIntFunction(func)) {
			emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CBF_CB_StringToInt"), 0, 0);
		}
		return;
	}

	if (name == "CB_StringToFloat") {
		if (!mStringValueType->setStringToFloatFunction(func)) {
			emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CBF_CB_StringToFloat"), 0, 0);
		}
		return;
	}
	if (name == "CB_IntToString") {
		if (!mStringValueType->setIntToStringFunction(func)) {
			emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CBF_CB_IntToString"), 0, 0);
		}
		return;
	}
	if (name == "CB_FloatToString") {
		if (!mStringValueType->setFloatToStringFunction(func)) {
			emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CBF_CB_FloatToString"), 0, 0);
		}
		return;
	}


}


Runtime *Runtime::instance() {
	return runtimeInstance;
}
