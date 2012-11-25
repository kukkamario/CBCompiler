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

	ecFunctionReturnTypeDefinedTwice,
	ecFunctionReturnTypeRequired,

	ecVarAlreadyDefinedWithAnotherType,

	ecParsingFailed,

	//Runtime
	ecCantLoadRuntime,
	ecInvalidRuntime,

	//Code generation
	ecNotConstant,
	ecConstantAlreadyDefined,
	ecSymbolAlreadyDefinedInRuntime,
	ecSymbolAlreadyDefined,
	ecCantFindType,
	ecForcingType,
	ecMathematicalOperationOperandTypeMismatch,
	ecCantCreateTypePointerLLVMStructType,

	ecTypeHasMultipleFieldsWithSameName,

	ecWTF

};
}
#endif // ERRORCODES_H
