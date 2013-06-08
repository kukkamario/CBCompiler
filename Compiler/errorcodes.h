#ifndef ERRORCODES_H
#define ERRORCODES_H

namespace ErrorCodes {
enum ErrorCode {
	ecLexicalAnalysingFailed,

	//Lexer
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
	ecCannotParseInteger,
	ecCannotParseFloat,
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

	//Code generation
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
	ecCantFindSymbol,
	ecNotCommand,
	ecExpectingValueAfterReturn,
	ecInvalidReturn,
	ecGosubCannotReturnValue,
	ecLabelAlreadyDefined,
	ecInvalidExit,

	ecCantWriteBitcodeFile,

	ecCodeGenerationFailed,

	ecTypeHasMultipleFieldsWithSameName,

	ecWTF

};
}
#endif // ERRORCODES_H
