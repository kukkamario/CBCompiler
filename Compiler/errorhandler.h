#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
#include <QString>
#include <QObject>
class QFile;
class ErrorHandler : public QObject
{

		Q_OBJECT
	public:
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
			ecExpectingWend,
			ecExpectingNext,
			ecExpectingTo,
			ecExpectingAssignment,
			ecCannotParseInteger,
			ecCannotParseFloat,
			ecForAndNextDontMatch,
			ecParsingFailed,

			ecVarAlreadyDefinedWithAnotherType
		};
		ErrorHandler();
	public slots:
		void warning(int code, QString msg, int line, QFile *file);
		void error(int code, QString msg, int line, QFile *file);
};

#endif // ERRORHANDLER_H
