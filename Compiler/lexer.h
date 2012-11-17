#ifndef LEXER_H
#define LEXER_H
#include <QList>
#include <QFile>
#include "token.h"
#include <QMap>
#include <QPair>
class Lexer: public QObject
{
		Q_OBJECT
	public:
		enum ReturnState {
			Success,
			ErrorButContinue,
			Error
		};

		Lexer();
		ReturnState tokenizeFile(const QString &file);

		void addToken(const Token &tok);
		void printTokens();
		void writeTokensToFile(const QString &fileName);
		QList<Token> tokens() const {return mTokens;}
		QList<QPair<QFile*, QString> > files() const {return mFiles; }
	private:
		ReturnState tokenize(const QString &file);
		QList<QPair<QFile*, QString> > mFiles; //File and code
		QList<Token> mTokens;
		QMap<QString, Token::Type> mKeywords;

		void combineTokens();

		ReturnState readToEOL(QString::const_iterator &i, const QString::const_iterator &end);
		ReturnState readToRemEnd(QString::const_iterator &i, const QString::const_iterator &end, int &line, QFile *file);
		ReturnState readFloatDot(QString::const_iterator &i, const QString::const_iterator &end, int line, QFile * file);
		ReturnState readNum(QString::const_iterator &i, const QString::const_iterator &end, int line, QFile * file);
		ReturnState readHex(QString::const_iterator &i, const QString::const_iterator &end, int line, QFile * file);
		ReturnState readString(QString::const_iterator &i, const QString::const_iterator &end, int &line, QFile * file);
		ReturnState readIdentifier(QString::const_iterator &i, const QString::const_iterator &end, int &line, QFile * file);
	signals:
		void warning(int code, QString msg, int line, QFile *file);
		void error(int code, QString msg, int line, QFile *file);
};

#endif // LEXER_H
