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
	ecCannotParseInteger,
	ecCannotParseFloat,
	ecForAndNextDontMatch,
	ecTypesDontMatch,
	ecUnexpectedToken,

	ecFunctionReturnTypeDefinedTwice,
	ecFunctionReturnTypeRequired,

	ecVarAlreadyDefinedWithAnotherType
};
}
#endif // ERRORCODES_H
