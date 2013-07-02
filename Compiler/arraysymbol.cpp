#include "arraysymbol.h"
#include "valuetype.h"
#include "builder.h"

ArraySymbol::ArraySymbol(const QString &name, ValueType *valType, int dim, QFile *file, int line):
	Symbol(name, file, line),
	mValueType(valType),
	mDimensions(dim),
	mGlobalArrayData(0),
	mGlobalIndexMultiplierArray(0)
{
}

QString ArraySymbol::info() const {
	return QString("Array \"%1\", %1").arg(mName);
}

void ArraySymbol::createGlobalVariables(Builder *builder) {
	mGlobalArrayData = new llvm::GlobalVariable(
				mValueType->llvmType()->getPointerTo(),
				false,
				llvm::GlobalValue::PrivateLinkage,
				llvm::ConstantPointerNull::get(mValueType->llvmType()->getPointerTo()),
				(name() + "_array_data").toStdString());

	if (mDimensions > 1) {
		mGlobalIndexMultiplierArray = new llvm::GlobalVariable(
					llvm::ArrayType::get(llvm::IntegerType::get(builder->context(), 32), mDimensions - 1),
					false,
					llvm::GlobalValue::PrivateLinkage,
					0,
					(name() + "_dimension_multipliers").toStdString());
	}
}
