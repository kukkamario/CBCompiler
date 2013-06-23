#ifndef LEXER_H
#define LEXER_H
#include <QList>
#include <QFile>
#include "token.h"
#include <QMap>
#include <QPair>
#include "settings.h"
#include <assert.h>

/**
 * @brief The Lexer class tokenizes a file and all files included.
 */
class Lexer: public QObject
{
		Q_OBJECT
	public:
		/**
		 * @brief The ReturnState enum
		 */
		enum ReturnState {
			Success,
			ErrorButContinue,
			Error
		};

		Lexer();
		/**
		 * @brief tokenizeFile tokenizes file and all files included inside it. Emits warning and error signals.
		 * @param file Path to file, relative to current directory.
		 * @param settings Settings
		 * @return ReturnState::Success, if the lexical analysis succeeded and ReturnState::Error or ReturnState::ErrorButContinue otherwise.
		 */
		ReturnState tokenizeFile(const QString &file, const Settings &settings);

		void addToken(const Token &tok);

		/**
		 * @brief printTokens Prints tokens to qDebug()
		 */
		void printTokens();

		/**
		 * @brief writeTokensToFile Outputs tokens to a file using an informative text format.
		 * @param fileName File in which tokens are written.
		 */
		void writeTokensToFile(const QString &fileName);

		/**
		 * @brief tokens
		 * @return List of the tokens
		 */
		QList<Token> tokens() const {return mTokens;}


		QList<QPair<QFile*, QString> > files() const {return mFiles; }
	private:

		ReturnState tokenize(const QString &file);
		QList<QPair<QFile*, QString> > mFiles; //File and code
		QList<Token> mTokens;
		QMap<QString, Token::Type> mKeywords;
		Settings mSettings;

		/**
		 * @brief combineTokens Combines tokens like "End If" to "EndIf".
		 */
		void combineTokens();

		ReturnState readToEOL(QString::iterator &i, const QString::iterator &end);
		ReturnState readToRemEnd(QString::iterator &i, const QString::iterator &end, int &line, QFile *file);
		ReturnState readFloatDot(QString::iterator &i, const QString::iterator &end, int line, QFile * file);
		ReturnState readNum(QString::iterator &i, const QString::iterator &end, int line, QFile * file);
		ReturnState readHex(QString::iterator &i, const QString::iterator &end, int line, QFile * file);
		ReturnState readString(QString::iterator &i, const QString::iterator &end, int &line, QFile * file);
		ReturnState readIdentifier(QString::iterator &i, const QString::iterator &end, int &line, QFile * file);
	signals:
		void warning(int code, QString msg, int line, QFile *file);
		void error(int code, QString msg, int line, QFile *file);
};

#endif // LEXER_H
