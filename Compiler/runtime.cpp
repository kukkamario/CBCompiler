#include "runtime.h"
#include <QDebug>
#include "intvaluetype.h"
#include "stringvaluetype.h"
#include "shortvaluetype.h"
#include "floatvaluetype.h"
#include "bytevaluetype.h"
#include "booleanvaluetype.h"
#include "typepointervaluetype.h"
#include "typevaluetype.h"
#include "errorcodes.h"
#include "settings.h"
#include "customdatatypedefinitions.h"
#include "customvaluetype.h"
#include <QStringList>
#include <QTextStream>

static Runtime *runtimeInstance = 0;

Runtime::Runtime():
	mValid(true),
	mModule(0),
	mCBMain(0),
	mCBInitialize(0),
	mIntValueType(0),
	mFloatValueType(0),
	mStringValueType(0),
	mShortValueType(0),
	mByteValueType(0),
	mBooleanValueType(0),
	mTypePointerCommonValueType(0),
	mDataLayout(0) {
	assert(runtimeInstance == 0);
	runtimeInstance = this;
}

Runtime::~Runtime() {
	runtimeInstance = 0;
	delete mBooleanValueType;
	delete mIntValueType;
	delete mShortValueType;
	delete mByteValueType;
	delete mStringValueType;
	delete mTypePointerCommonValueType;
	delete mFloatValueType;
}

bool Runtime::load(StringPool *strPool, const Settings &settings) {
	llvm::InitializeNativeTarget();
	llvm::SMDiagnostic diagnostic;
	mModule = llvm::ParseIRFile(settings.runtimeLibraryPath().toStdString(), diagnostic, llvm::getGlobalContext());
	if (!mModule) {
		emit error(ErrorCodes::ecCantLoadRuntime, "Runtime loading failed: " + QString::fromStdString(diagnostic.getMessage()), 0, QString());
		return false;
	}

	if (!loadFunctionMapping(settings.functionMappingFile())) return false;

	mDataLayout = new llvm::DataLayout(mModule);

	if (!loadValueTypes(strPool)) return false;

	if (!loadCustomDataTypes(settings.dataTypesFile())) return false;

	if (!generateLLVMValueTypeMapping()) return false;

	if (!loadRuntimeFunctions()) return false;

	loadDefaultRuntimeFunctions();



	if (!(mStringValueType && mByteValueType && mShortValueType && mIntValueType && mFloatValueType)) return false;
	if (!(mStringValueType->isValid() && mTypeValueType->isValid())) return false;
	if (!(mCBMain && mCBInitialize)) return false;
	return mValid;
}


bool Runtime::loadValueTypes(StringPool *strPool) {
	//Int
	mIntValueType = new IntValueType(this, mModule);
	mValueTypes.append(mIntValueType);

	//String
	mStringValueType = new StringValueType(strPool, this, mModule);
	llvm::StructType *str = mModule->getTypeByName("struct.CB_StringData");
	if (!str) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Can't find \"struct.CB_StringData\" in runtime library bitcode"), 0, QString());
		return false;
	}

	mTypeLLVMType = mModule->getTypeByName("struct.CB_Type");
	if (!mTypeLLVMType) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Can't find \"struct.CB_Type\" in runtime library bitcode"), 0, QString());
		return false;
	}

	mTypeMemberLLVMType = mModule->getTypeByName("struct.CB_TypeMember");
	if (!mTypeMemberLLVMType) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Can't find \"struct.CB_TypeMember\" in runtime library bitcode"), 0, QString());
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

	mTypePointerCommonValueType = new TypePointerCommonValueType(this, typeMemberPointerLLVMType());
	mValueTypes.append(mTypePointerCommonValueType);

	mTypeValueType = new TypeValueType(this, typeLLVMType()->getPointerTo());
	mValueTypes.append(mTypeValueType);

	mValueTypeEnum[ValueType::Integer] = intValueType();
	mValueTypeEnum[ValueType::Float] = floatValueType();
	mValueTypeEnum[ValueType::String]  = stringValueType();
	mValueTypeEnum[ValueType::Short] = shortValueType();
	mValueTypeEnum[ValueType::Byte] = byteValueType();
	mValueTypeEnum[ValueType::Boolean] = booleanValueType();
	mValueTypeEnum[ValueType::TypePointerCommon] = typePointerCommonValueType();
	mValueTypeEnum[ValueType::Type] = typeValueType();
	return true;
}

bool Runtime::isAllocatorFunctionValid() {
	if (!mAllocatorFunction) return false;
	if (mAllocatorFunction->arg_size() != 1) return false;
	if (mAllocatorFunction->arg_begin()->getType() != llvm::IntegerType::get(mModule->getContext(), 32)) return false;
	return mAllocatorFunction->getReturnType() == llvm::IntegerType::get(mModule->getContext(), 8)->getPointerTo();
}

bool Runtime::isFreeFuntionValid() {
	if (!mFreeFunction) return false;
	if (mFreeFunction->arg_size() != 1) return false;
	if (mFreeFunction->arg_begin()->getType() != llvm::IntegerType::get(mModule->getContext(), 8)->getPointerTo()) return false;
	return mFreeFunction->getReturnType() == llvm::Type::getVoidTy(mModule->getContext());
}

bool Runtime::loadFunctionMapping(const QString &functionMapping) {
	QFile file(functionMapping);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		emit error(ErrorCodes::ecCantLoadFunctionMapping, tr("Can't open a function mapping file \"%1\"").arg(functionMapping), 0, QString());
		return false;
	}
	QTextStream in(&file);
	int lineNum = 0;
	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		lineNum++;
		if (line.isEmpty()) continue;
		QStringList parts = line.split('=');
		if (parts.size() != 2) {
			emit error(ErrorCodes::ecInvalidFunctionMappingFile, tr("Invalid function mapping file"), lineNum, functionMapping);
			return false;
		}
		mFunctionMapping.insert(parts.first(), parts.last());
	}
	file.close();
	return true;
}

bool Runtime::loadCustomDataTypes(const QString &customDataTypes) {
	CustomDataTypeDefinitions defs;
	connect(&defs, &CustomDataTypeDefinitions::error, this, &Runtime::error);
	connect(&defs, &CustomDataTypeDefinitions::error, this, &Runtime::error);

	if (!defs.parse(customDataTypes)) return false;

	bool valid = true;
	foreach (const CustomDataTypeDefinitions::CustomDataType &dt, defs.dataTypes()) {
		int pointerLevel = 0;
		QString dtName = dt.mDataType;
		while (dtName.endsWith('*')) {
			dtName.chop(1);
			pointerLevel++;
		}

		llvm::Type *type = mModule->getTypeByName(dtName.toStdString());
		if (!type) {
			emit error(ErrorCodes::ecCantFindCustomDataType, tr("Can't find a custom data type \"%1\".").arg(dtName), 0, customDataTypes);
			valid = false;
			continue;
		}

		while (pointerLevel--) { type = type->getPointerTo(); }

		CustomValueType *valTy = new CustomValueType(dt.mName.toLower(), type, this);
		mCustomValueTypes.append(valTy);
		mValueTypes.append(valTy);
	}
	return valid;
}

bool Runtime::generateLLVMValueTypeMapping() {
	bool valid = true;
	foreach( ValueType *valTy, mValueTypes) {
		if (mLLVMValueTypeMapping.contains(valTy->llvmType())) {
			emit error(ErrorCodes::ecValueTypeDefinedMultipleTimes, tr("Value type mapping \"%1\": There is already a value type \"%2\" with same llvm::Type. ").arg(valTy->name(), mLLVMValueTypeMapping[valTy->llvmType()]->name()), 0, QString());
			valid = false;
			continue;
		}
		mLLVMValueTypeMapping[valTy->llvmType()] = valTy;
	}
	return valid;
}


Runtime *Runtime::instance() {
	return runtimeInstance;
}


ValueType *Runtime::findValueType(ValueType::eType valType) {
	ValueType *vt = mValueTypeEnum[valType];
	assert(vt);
	return vt;
}

ValueType *Runtime::findValueType(llvm::Type *llvmType) {
	return mLLVMValueTypeMapping.value(llvmType);
}

bool Runtime::loadRuntimeFunctions() {
	bool valid = true;
	for (QMultiMap<QString, QString>::ConstIterator i = mFunctionMapping.begin(); i != mFunctionMapping.end(); i++) {
		RuntimeFunction *rtFunc = new RuntimeFunction(this);
		llvm::Function *func = mModule->getFunction(i.value().toStdString());
		if (!func) {
			emit error(ErrorCodes::ecCantFindRuntimeFunction, tr("Can't find the runtime function \"%1\", the mangled name \"%2\"").arg(i.key(), i.value()), 0, QString());
			valid = false;
			continue;
		}
		if(!rtFunc->construct(func, i.key())) {
			emit error(ErrorCodes::ecInvalidRuntime, tr("Invalid runtime function \"%1\" \"%2\"").arg(i.key(), i.value()), 0, QString());
			valid = false;
			continue;
		}
		mFunctions.append(rtFunc);
	}
	return valid;
}

bool Runtime::loadDefaultRuntimeFunctions() {
	llvm::Function *func = 0;

	mCBMain = mModule->getFunction("CB_main");
	if (!mCBMain) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Can't find \"CB_main\""), 0, QString());
	}

	mCBInitialize = mModule->getFunction("CB_initialize");
	if (!mCBInitialize) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Can't find \"CB_initialize\""), 0, QString());
	}

	func = mModule->getFunction("CB_StringConstruct");
	if (!func || !mStringValueType->setConstructFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_StringConstruct"), 0, QString());
	}

	func = mModule->getFunction("CB_StringDestruct");
	if (!func || !mStringValueType->setDestructFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_StringDestruct"), 0, QString());
	}

	func = mModule->getFunction("CB_StringAssign");
	if (!func || !mStringValueType->setAssignmentFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_StringAssign"), 0, QString());
	}

	func = mModule->getFunction("CB_StringToInt");
	if (!func || !mStringValueType->setStringToIntFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_StringToInt"), 0, QString());
	}

	func = mModule->getFunction("CB_StringToFloat");
	if (!func || !mStringValueType->setStringToFloatFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_StringToFloat"), 0, QString());
	}

	func = mModule->getFunction("CB_IntToString");
	if (!func || !mStringValueType->setIntToStringFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_IntToString"), 0, QString());
	}

	func = mModule->getFunction("CB_FloatToString");
	if (!func || !mStringValueType->setFloatToStringFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_FloatToString"), 0, QString());
	}

	func = mModule->getFunction("CB_StringAddition");
	if (!func || !mStringValueType->setAdditionFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_StringAddition"), 0, QString());
	}

	func = mModule->getFunction("CB_StringEquality");
	if (!func || !mStringValueType->setEqualityFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_StringEquality"), 0, QString());
	}

	func = mModule->getFunction("CB_StringRef");
	if (!func || !mStringValueType->setRefFunction(func)) {
		mValid = false;
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_RefString"), 0, QString());
	}

	mAllocatorFunction = mModule->getFunction("CB_Allocate");
	if (!isAllocatorFunctionValid()) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_Allocate"), 0, QString());
		mValid = false;
	}

	mFreeFunction = mModule->getFunction("CB_Free");
	if (!isFreeFuntionValid()) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_Free"), 0, QString());
		mValid = false;
	}

	func = mModule->getFunction("CB_ConstructType");
	if (!func || !mTypeValueType->setConstructTypeFunction(func)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_ConstructType"), 0, QString());
		mValid = false;
	}

	func = mModule->getFunction("CB_New");
	if (!func || !mTypeValueType->setNewFunction(func)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_New"), 0, QString());
		mValid = false;
	}

	func = mModule->getFunction("CB_First");
	if (!func || !mTypeValueType->setFirstFunction(func)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_First"), 0, QString());
		mValid = false;
	}

	func = mModule->getFunction("CB_Last");
	if (!func || !mTypeValueType->setLastFunction(func)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_Last"), 0, QString());
		mValid = false;
	}

	func = mModule->getFunction("CB_Before");
	if (!func || !mTypeValueType->setBeforeFunction(func)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_Before"), 0, QString());
		mValid = false;
	}

	func = mModule->getFunction("CB_After");
	if (!func || !mTypeValueType->setAfterFunction(func)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("RUNTIME: Invalid CB_After"), 0, QString());
		mValid = false;
	}

	return mValid;
}
