#include "runtime.h"
#include "intvaluetype.h"
#include "stringvaluetype.h"
#include "shortvaluetype.h"
#include "floatvaluetype.h"
#include "bytevaluetype.h"
#include "booleanvaluetype.h"
#include "typepointervaluetype.h"
#include "typevaluetype.h"
#include "genericarrayvaluetype.h"
#include "errorcodes.h"
#include "settings.h"
#include "customdatatypedefinitions.h"
#include "customvaluetype.h"
#include "genericstructvaluetype.h"
#include "nullvaluetype.h"
#include <fstream>
#include <time.h>
#include <boost/algorithm/string.hpp>
#include "errorhandler.h"
#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

static Runtime *runtimeInstance = 0;

Runtime::Runtime(ErrorHandler *errorHandler):
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
	mValueTypeCollection(this),
	mDataLayout(0),
	mErrorHandler(errorHandler) {
	assert(runtimeInstance == 0);
	runtimeInstance = this;
}

Runtime::~Runtime() {
	runtimeInstance = 0;
	delete mTypePointerCommonValueType;
}

bool Runtime::load(StringPool *strPool, Settings *settings) {
	llvm::InitializeNativeTarget();
	llvm::SMDiagnostic diagnostic;
	std::string path = settings->runtimeLibraryPath();
	mModule = llvm::ParseIRFile(path, diagnostic, llvm::getGlobalContext());
	if (!mModule) {
		error(ErrorCodes::ecCantLoadRuntime, ("Runtime loading failed: " + diagnostic.getMessage()).str(), CodePoint());
		return false;
	}

	if (!loadFunctionMapping(settings->functionMappingFile())) return false;

	mDataLayout = new llvm::DataLayout(mModule);

	if (!loadValueTypes(strPool)) return false;

	if (!loadCustomDataTypes(settings->dataTypesFile())) return false;

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
	mValueTypeCollection.addValueType(mIntValueType);

	//String
	mStringValueType = new StringValueType(strPool, this);
	llvm::StructType *str = mModule->getTypeByName("class.LString");
	if (!str) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Can't find \"class.LString\" in runtime library bitcode", CodePoint());
		return false;
	}

	mTypeLLVMType = mModule->getTypeByName("struct.CB_Type");
	if (!mTypeLLVMType) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Can't find \"struct.CB_Type\" in runtime library bitcode", CodePoint());
		return false;
	}

	mTypeMemberLLVMType = mModule->getTypeByName("struct.CB_TypeMember");
	if (!mTypeMemberLLVMType) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Can't find \"struct.CB_TypeMember\" in runtime library bitcode", CodePoint());
		return false;
	}

	mGenericArrayLLVMType = mModule->getTypeByName("struct.CB_GenericArrayDataHeader");
	if (!mGenericArrayLLVMType) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Can't find \"struct.CB_GenericArrayDataHeader\" in runtime library bitcode", CodePoint());
		return false;
	}

	mGenericStructLLVMType = mModule->getTypeByName("struct.CB_GenericStruct");
	if (!mGenericStructLLVMType) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Can't find \"struct.CB_GenericStruct\" in runtime library bitcode", CodePoint());
		return false;
	}

	mStringValueType->setStringType(str);
	mValueTypeCollection.addValueType(mStringValueType);

	mFloatValueType = new FloatValueType(this, mModule);
	mValueTypeCollection.addValueType(mFloatValueType);

	mShortValueType = new ShortValueType(this, mModule);
	mValueTypeCollection.addValueType(mShortValueType);

	mByteValueType = new ByteValueType(this, mModule);
	mValueTypeCollection.addValueType(mByteValueType);

	mBooleanValueType = new BooleanValueType(this, mModule);
	mValueTypeCollection.addValueType(mBooleanValueType);

	mTypePointerCommonValueType = new TypePointerCommonValueType(this, typeMemberPointerLLVMType());
	mValueTypeCollection.addValueType(mTypePointerCommonValueType);

	mTypeValueType = new TypeValueType(this, typeLLVMType()->getPointerTo());
	mValueTypeCollection.addValueType(mTypeValueType);

	mGenericArrayValueType = new GenericArrayValueType(mGenericArrayLLVMType->getPointerTo(), this);
	mValueTypeCollection.addValueType(mGenericArrayValueType);

	mGenericStructValueType = new GenericStructValueType(mGenericStructLLVMType->getPointerTo(), this);
	mValueTypeCollection.addValueType(mGenericStructValueType);

	mNullValueType = new NullValueType(this);
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

void Runtime::warning(int code, std::string msg, CodePoint cp) {
	mErrorHandler->warning(code, msg, cp);
}

void Runtime::error(int code, std::string msg, CodePoint cp) {
	mErrorHandler->error(code, msg, cp);
}

bool Runtime::loadFunctionMapping(const std::string &functionMapping) {
	std::ifstream file(functionMapping);
	if (!file.is_open()) {
		error(ErrorCodes::ecCantLoadFunctionMapping, "Can't open a function mapping file \"" + functionMapping + "\"", CodePoint());
		return false;
	}
	int lineNum = 0;
	std::string line;
	while (std::getline(file, line)) {
		boost::algorithm::trim(line);
		lineNum++;
		if (line.empty()) continue;
		std::vector<std::string> parts;
		boost::algorithm::split(parts, line, boost::algorithm::is_from_range('=', '='), boost::algorithm::token_compress_on);
		if (parts.size() != 2) {
			error(ErrorCodes::ecInvalidFunctionMappingFile, "Invalid function mapping file", CodePoint(lineNum, 1, functionMapping));
			return false;
		}
		mFunctionMapping.insert(std::pair<std::string, std::string>(parts.front(), parts.back()));
	}
	file.close();
	return true;
}

bool Runtime::loadCustomDataTypes(const std::string &customDataTypes) {
	CustomDataTypeDefinitions defs(mErrorHandler);

	if (!defs.parse(customDataTypes)) return false;

	bool valid = true;
	for (const CustomDataTypeDefinitions::CustomDataType &dt : defs.dataTypes()) {
		int pointerLevel = 0;
		std::string dtName = dt.mDataType;
		while (boost::algorithm::ends_with(dtName, "*")) {
			dtName.erase(--dtName.end(), dtName.end());
			pointerLevel++;
		}
		llvm::Type *type = mModule->getTypeByName(dtName);
		if (!type) {
			error(ErrorCodes::ecCantFindCustomDataType, "Can't find a custom data type \"" + dtName + "\"", CodePoint(0, 0, customDataTypes));
			valid = false;
			continue;
		}

		while (pointerLevel--) { type = type->getPointerTo(); }

		CustomValueType *valTy = new CustomValueType(boost::locale::to_lower(dt.mName), type, this);
		mValueTypeCollection.addValueType(valTy);
	}
	return valid;
}


Runtime *Runtime::instance() {
	return runtimeInstance;
}


bool Runtime::loadRuntimeFunctions() {
	bool valid = true;
	for (std::multimap<std::string, std::string>::const_iterator i = mFunctionMapping.begin(); i != mFunctionMapping.end(); i++) {
		RuntimeFunction *rtFunc = new RuntimeFunction(this);
		llvm::Function *func = mModule->getFunction(i->second);
		if (!func) {
			error(ErrorCodes::ecCantFindRuntimeFunction, (boost::format("Can't find the runtime function \"%1%\", the mangled name \"%2%\"") % i->first % i->second).str(), CodePoint());
			valid = false;
			continue;
		}
		if(!rtFunc->construct(func, i->first)) {
			error(ErrorCodes::ecInvalidRuntime, (boost::format("Invalid runtime function \"%1%\" \"%2%\"") % i->first % i->second).str(), CodePoint());
			valid = false;
			continue;
		}
		mFunctions.push_back(rtFunc);
	}
	return valid;
}

bool Runtime::loadDefaultRuntimeFunctions() {
	llvm::Function *func = 0;

	mCBMain = mModule->getFunction("CB_main");
	if (!mCBMain) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Can't find \"CB_main\"", CodePoint());
	}

	mCBInitialize = mModule->getFunction("CB_initialize");
	if (!mCBInitialize) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Can't find \"CB_initialize\"", CodePoint());
	}

	func = mModule->getFunction("_Z18CB_StringConstructPDi");
	if (!func || !mStringValueType->setConstructFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_StringConstruct", CodePoint());
	}

	func = mModule->getFunction("_Z17CB_StringDestructP11LStringData");
	if (!func || !mStringValueType->setDestructFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_StringDestruct", CodePoint());
	}

	func = mModule->getFunction("_Z15CB_StringAssignPP11LStringDataS0_");
	if (!func || !mStringValueType->setAssignmentFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_StringAssign", CodePoint());
	}

	func = mModule->getFunction("_Z14CB_StringToIntP11LStringData");
	if (!func || !mStringValueType->setStringToIntFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_StringToInt", CodePoint());
	}

	func = mModule->getFunction("_Z16CB_StringToFloatP11LStringData");
	if (!func || !mStringValueType->setStringToFloatFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_StringToFloat", CodePoint());
	}

	func = mModule->getFunction("_Z14CB_IntToStringi");
	if (!func || !mStringValueType->setIntToStringFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_IntToString", CodePoint());
	}

	func = mModule->getFunction("_Z16CB_FloatToStringf");
	if (!func || !mStringValueType->setFloatToStringFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_FloatToString", CodePoint());
	}

	func = mModule->getFunction("_Z17CB_StringAdditionP11LStringDataS0_");
	if (!func || !mStringValueType->setAdditionFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_StringAddition", CodePoint());
	}

	func = mModule->getFunction("_Z17CB_StringEqualityP11LStringDataS0_");
	if (!func || !mStringValueType->setEqualityFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_StringEquality", CodePoint());
	}

	func = mModule->getFunction("_Z12CB_StringRefP11LStringData");
	if (!func || !mStringValueType->setRefFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_StringRef", CodePoint());
	}

	func = mModule->getFunction("CB_ArrayRef");
	if (!func || !mGenericArrayValueType->setRefFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_ArrayRef", CodePoint());
	}

	func = mModule->getFunction("CB_ArrayConstruct");
	if (!func || !mGenericArrayValueType->setConstructFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_ArrayConstruct", CodePoint());
	}

	func = mModule->getFunction("CB_ArrayDestruct");
	if (!func || !mGenericArrayValueType->setDestructFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_ArrayDestruct", CodePoint());
	}

	func = mModule->getFunction("CB_ArrayAssign");
	if (!func || !mGenericArrayValueType->setAssignmentFunction(func)) {
		mValid = false;
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_ArrayAssign", CodePoint());
	}

	mAllocatorFunction = mModule->getFunction("CB_Allocate");
	if (!isAllocatorFunctionValid()) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_Allocate", CodePoint());
		mValid = false;
	}

	mFreeFunction = mModule->getFunction("CB_Free");
	if (!isFreeFuntionValid()) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_Free", CodePoint());
		mValid = false;
	}

	func = mModule->getFunction("CB_ConstructType");
	if (!func || !mTypeValueType->setConstructTypeFunction(func)) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_ConstructType", CodePoint());
		mValid = false;
	}

	func = mModule->getFunction("CB_New");
	if (!func || !mTypeValueType->setNewFunction(func)) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_New", CodePoint());
		mValid = false;
	}

	func = mModule->getFunction("CB_First");
	if (!func || !mTypeValueType->setFirstFunction(func)) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_First", CodePoint());
		mValid = false;
	}

	func = mModule->getFunction("CB_Last");
	if (!func || !mTypeValueType->setLastFunction(func)) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_Last", CodePoint());
		mValid = false;
	}

	func = mModule->getFunction("CB_Before");
	if (!func || !mTypeValueType->setBeforeFunction(func)) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_Before", CodePoint());
		mValid = false;
	}

	func = mModule->getFunction("CB_After");
	if (!func || !mTypeValueType->setAfterFunction(func)) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_After", CodePoint());
		mValid = false;
	}

	func = mModule->getFunction("CB_Delete");
	if (!func || !mTypeValueType->setDeleteFunction(func)) {
		error(ErrorCodes::ecInvalidRuntime, "RUNTIME: Invalid CB_Delete", CodePoint());
		mValid = false;
	}

	return mValid;
}
