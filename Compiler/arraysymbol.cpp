#include "arraysymbol.h"
#include "valuetype.h"
#include "builder.h"

ArraySymbol::ArraySymbol(const QString &name, ValueType *valType, int dim, const QString &file, int line):
	Symbol(name, file, line),
	mValueType(valType),
	mDimensions(dim),
	mGlobalArrayData(0),
	mGlobalIndexMultiplierArray(0),
	mGlobalArraySize(0)
{
}

QString ArraySymbol::info() const {
	return QString("Array \"%1\", %1").arg(mName);
}

void ArraySymbol::createGlobalVariables(Builder *builder) {
	mGlobalArrayData = builder->createGlobalVariable(
				mValueType->llvmType()->getPointerTo(),
				false,
				llvm::GlobalValue::CommonLinkage,
				llvm::ConstantPointerNull::get(mValueType->llvmType()->getPointerTo()),
				(name() + "_array_data").toStdString());

	mGlobalArraySize = builder->createGlobalVariable(
				llvm::IntegerType::get(builder->context(), 32),
				false,
				llvm::GlobalValue::CommonLinkage,
				llvm::Constant::getNullValue(llvm::IntegerType::get(builder->context(), 32)),
				(name() + "_array_size").toStdString());

	if (mDimensions > 1) {
		llvm::ArrayType *type = llvm::ArrayType::get(llvm::IntegerType::get(builder->context(), 32), mDimensions - 1);
		mGlobalIndexMultiplierArray = builder->createGlobalVariable(
					type,
					false,
					llvm::GlobalValue::CommonLinkage,
					llvm::Constant::getNullValue(type),
					(name() + "_dimension_multipliers").toStdString());
	}

}
