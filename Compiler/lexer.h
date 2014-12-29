#ifndef LEXER_H
#define LEXER_H
#include <vector>
#include <QFile>
#include "token.h"
#include <map>
#include <utility>
#include "settings.h"
#include <cassert>
#include "utf8.h"
#include "errorhandler.h"

typedef utf8::iterator<std::string::const_iterator> utf8_iterator;
/**
 * @brief The Lexer class tokenizes a file and all files included.
 */
class Lexer {
	public:
		/**
		 * @brief The ReturnState enum
		 */
		enum ReturnState {
			Success,
			ErrorButContinue,
			Error
		};

		Lexer(ErrorHandler *errorHandler);
		/**
		 * @brief tokenizeFile tokenizes file and all files included inside it. Emits warning and error signals.
		 * @param file Path to file, relative to current directory.
		 * @param settings Settings
		 * @return ReturnState::Success, if the lexical analysis succeeded and ReturnState::Error or ReturnState::ErrorButContinue otherwise.
		 */
		ReturnState tokenizeFile(const std::string &file, Settings *settings);

		void addToken(const Token &tok);
		void addToken(Token::Type type, utf8_iterator begin, utf8_iterator end, const CodePoint &cp);

		/**
		 * @brief printTokens Prints tokens to qDebug()
		 */
		void printTokens();

		/**
		 * @brief writeTokensToFile Outputs tokens to a file using an informative text format.
		 * @param fileName File in which tokens are written.
		 */
		void writeTokensToFile(const std::string &fileName);

		/**
		 * @brief tokens
		 * @return List of the tokens
		 */
		std::vector<Token> tokens() const {return mTokens;}


		std::vector<std::pair<std::string, std::string> > files() const {return mFiles; }
	private:
		/**
		 * @brief combineTokens Combines tokens like "End If" to "EndIf".
		 */
		void combineTokens();

		ReturnState readToEOL(utf8_iterator &i, const utf8_iterator &end);
		ReturnState readToEndRem(utf8_iterator  &i, const utf8_iterator &end, utf8_iterator &lineStart, int &line, const std::string &file);
		ReturnState readFloatDot(utf8_iterator  &i, const utf8_iterator &end, utf8_iterator &lineStart, int line, const std::string &file);
		ReturnState readNum(utf8_iterator  &i, const utf8_iterator &end, utf8_iterator &lineStart, int line, const std::string &file);
		ReturnState readHex(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int line, const std::string &file);
		ReturnState readString(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int &line, const std::string &file);
		ReturnState readIdentifier(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int &line, const std::string &file);

		CodePoint codePoint(utf8_iterator i, utf8_iterator lineStart, int line, const std::string &file);

		static bool isDigit(utf8_iterator i);
		static bool isAlnum(utf8_iterator i);
		static bool isAlpha(utf8_iterator i);
		static bool isSpace(utf8_iterator i);

		void error(int code, std::string msg, CodePoint cp);
		void warning(int code, std::string msg, CodePoint cp);

		ReturnState tokenize(const std::string &file);
		std::vector<std::pair<std::string, std::string> > mFiles; //File and code
		std::vector<Token> mTokens;
		std::map<std::string, Token::Type> mKeywords;
		Settings *mSettings;
		ErrorHandler *mErrorHandler;
};

#endif // LEXER_H
