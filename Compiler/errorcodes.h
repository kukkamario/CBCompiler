#ifndef ERRORCODES_H
#define ERRORCODES_H

namespace ErrorCodes {
enum ErrorCode {
	//Settings
	ecNoInputFiles = 11,
	ecSettingsLoadingFailed,
	ecCantParseCustomDataTypeDefinitionFile,
	ecInvalidCustomDataTypeDefinitionFileFormat,

	//Lexer
	ecLexicalAnalysingFailed,
	ecCantOpenFile,
	ecExpectingEndOfString,
	ecExpectingRemEndBeforeEOF,
	ecExpectingString,
	ecUnexpectedCharacter,

	//Parser
	ecExpectingIdentifier,
	ecExpectingEndOfStatement,
	ecExpectingVarType,
	ecExpectingLeftParenthese,
	ecExpectingRightParenthese,
	ecExpectingLeftSquareBracket,
	ecExpectingRightSquareBracket,
	ecExpectingPrimaryExpression,
	ecExpectingEndOfRepeat,
	ecExpectingEndFunction,
	ecExpectingEndType,
	ecExpectingEndIf,
	ecExpectingWend,
	ecExpectingNext,
	ecExpectingTo,
	ecExpectingEach,
	ecExpectingTypePtr,
	ecExpectingVariable,
	ecExpectingAssignment,
	ecExpectingEndSelect,
	ecCantParseInteger,
	ecCantParseFloat,
	ecForAndNextDontMatch,
	ecTypesDontMatch,
	ecUnexpectedToken,
	ecMultipleSelectDefaultCases,
	ecExpectingLabel,
	ecExpectingFunctionPointerType,
	ecExpectingParameterTypeList,
	ecExpectingArraySubscript,

	ecFunctionReturnTypeDefinedTwice,
	ecFunctionReturnTypeRequired,

	ecVariableAlreadyDefinedWithAnotherType,
	ecConstantAlreadyDefinedWithAnotherType,
	ecVariableTypeDefinedTwice,

	ecParsingFailed,

	//Runtime
	ecCantLoadRuntime,
	ecInvalidRuntime,
	ecPrefixReserved,
	ecCantLoadFunctionMapping,
	ecInvalidFunctionMappingFile,
	ecCantFindRuntimeFunction,
	ecCantFindCustomDataType,
	ecValueTypeDefinedMultipleTimes,

	//Code generation
	ecCodeGeneratorInitializationFailed,
	ecOnlyBasicValueTypeCanBeConstant,
	ecNotConstant,
	ecNotVariable,
	ecNotTypePointer,
	ecNotLabel,
	ecArraySubscriptNotInteger,
	ecConstantAlreadyDefined,
	ecSymbolAlreadyDefinedInRuntime,
	ecSymbolAlreadyDefined,
	ecCantFindType,
	ecCantFindField,
	ecForEachInvalidContainer,
	ecForcingType,
	ecNotArray,
	ecInvalidArraySubscript,
	ecMathematicalOperationOperandTypeMismatch,
	ecCantCreateTypePointerLLVMStructType,
	ecVariableNotDefined,
	ecCantFindFunction,
	ecCantFindCommand,
	ecMultiplePossibleOverloads,
	ecNoCastFromTypePointer,
	ecNotNumber,
	ecInvalidAssignment,
	ecVariableAlreadyDefined,
	ecParametreSymbolAlreadyDefined,
	ecFunctionAlreadyDefined,
	ecSymbolAlreadyDefinedWithDifferentType,
	ecCantFindSymbol,
	ecForVariableContainerTypeMismatch,
	ecNotCommand,
	ecExpectingValueAfterReturn,
	ecInvalidReturn,
	ecGosubCannotReturnValue,
	ecLabelAlreadyDefined,
	ecInvalidExit,
	ecCantUseCommandInsideExpression,
	ecSymbolNotArray,
	ecSymbolNotArrayOrCommand,
	ecArrayDimensionCountDoesntMatch,
	ecExpectingNumberValue,
	ecDangerousFloatToIntCast,
	ecTypeCantHaveValueType,
	ecExpectingType,
	ecCastFunctionRequiresOneParameter,
	ecCantCastValue,
	ecExpectingVariableDefinition,
	ecSymbolNotValueType,
	ecIntegerDividedByZero,
	ecOperationNotAllowedInConstantExpression,
	ecWrongNumberOfParameters,
	ecNotTypeName,
	ecReferenceRequired,
	ecNotContainer,
	ecEmptySelect,
	ecStructContainsItself,
	ecNotInteger,
	ecInvalidParameter,
	ecReturnParameterInCommand,
	ecFunctionSelectorParametersShouldBeValueTypes,

	ecCantWriteBitcodeFile,
	ecOptimizingFailed,
	ecCantCreateObjectFile,
	ecNativeLinkingFailed,

	ecCodeGenerationFailed,

	ecTypeHasMultipleFieldsWithSameName,

	ecWTF

};
}
#endif // ERRORCODES_H
