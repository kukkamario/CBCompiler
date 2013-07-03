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
	mGlobalArrayData = builder->createGlobalVariable(
				mValueType->llvmType()->getPointerTo(),
				false,
				llvm::GlobalValue::CommonLinkage,
				llvm::ConstantPointerNull::get(mValueType->llvmType()->getPointerTo()),
				(name() + "_array_data").toStdString());

	if (mDimensions > 1) {
		//Initialize a global array filled with zeros
		llvm::ArrayType *type = llvm::ArrayType::get(llvm::IntegerType::get(builder->context(), 32), mDimensions - 1);
		std::vector<llvm::Constant*> zeros(mDimensions - 1, builder->irBuilder().getInt32(0));
		llvm::Constant *zeroArr = llvm::ConstantArray::get(type, zeros);
		mGlobalIndexMultiplierArray = builder->createGlobalVariable(
					type,
					false,
					llvm::GlobalValue::CommonLinkage,
					zeroArr,
					(name() + "_dimension_multipliers").toStdString());
	}
}
