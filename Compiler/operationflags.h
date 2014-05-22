#ifndef OPERATIONFLAGS_H
#define OPERATIONFLAGS_H
#include <QFlags>

namespace OperationFlag {
	enum OperationFlagEnum {
		NoFlags = 0x00,
		MayLosePrecision = 0x01,
		IntegerDividedByZero = 0x02,
		NoSuchOperation = 0x04,
		CastToString = 0x08,
		CastFromString = 0x10,
		OperandBCantBeCastedToA = 0x20,
		ReferenceRequired = 0x30
	};

}

Q_DECLARE_FLAGS(OperationFlags, OperationFlag::OperationFlagEnum)

bool operationFlagsContainFatalFlags(OperationFlags flags) {
	return (flags & (OperationFlag::IntegerDividedByZero | OperationFlag::NoSuchOperation | OperationFlag::ReferenceRequired | OperationFlag::OperandBCantBeCastedToA)) != 0;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(OperationFlags)

#endif // OPERATIONFLAGS_H
