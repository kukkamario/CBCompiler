#ifndef ERRORCODES_H
#define ERRORCODES_H

namespace ErrorCodes {
enum ErrorCode {
	//Settings
	ecSettingsLoadingFailed = 11,

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

	ecFunctionReturnTypeDefinedTwice,
	ecFunctionReturnTypeRequired,

	ecVarAlreadyDefinedWithAnotherType,

	ecParsingFailed,

	//Runtime
	ecCantLoadRuntime,
	ecInvalidRuntime,
	ecPrefixReserved,

	//Code generation
	ecCodeGeneratorInitializationFailed,
	ecNotConstant,
	ecNotVariable,
	ecNotTypePointer,
	ecArraySubscriptNotInteger,
	ecConstantAlreadyDefined,
	ecSymbolAlreadyDefinedInRuntime,
	ecSymbolAlreadyDefined,
	ecCantFindType,
	ecCantFindTypeField,
	ecForcingType,
	ecNotArrayOrFunction,
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

	ecCantWriteBitcodeFile,

	ecCodeGenerationFailed,

	ecTypeHasMultipleFieldsWithSameName,

	ecWTF

};
}
#endif // ERRORCODES_H
