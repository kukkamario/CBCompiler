#include <QCoreApplication>
#include <QStringList>
#include "lexer.h"
#include <QDebug>
#include <QTime>
#include "errorhandler.h"
#include "parser.h"
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QStringList params = a.arguments();
	if (params.size() != 2) {
		qCritical() << "Expecting 2 parametres";
		return 0;
	}
	ErrorHandler errHandler;
	Lexer lexer;
	errHandler.connect(&lexer, SIGNAL(error(int,QString,int,QFile*)), SLOT(error(int,QString,int,QFile*)));
	errHandler.connect(&lexer, SIGNAL(warning(int,QString,int,QFile*)), SLOT(warning(int,QString,int,QFile*)));
	QTime startTime = QTime::currentTime();
	if (lexer.tokenizeFile(params.last()) == Lexer::Success) {
		qDebug() << "Lexical analysing took " << startTime.msecsTo(QTime::currentTime()) << "ms";
		lexer.writeTokensToFile("tokens.txt");
	}
	else {
		errHandler.error(ErrorHandler::ecLexicalAnalysingFailed, errHandler.tr("Lexical analysing failed"), 0, lexer.files().first().first);
		return 1;
	}

	Parser parser;
	errHandler.connect(&parser, SIGNAL(error(int,QString,int,QFile*)), SLOT(error(int,QString,int,QFile*)));
	errHandler.connect(&parser, SIGNAL(warning(int,QString,int,QFile*)), SLOT(warning(int,QString,int,QFile*)));
	bool error;
	ast::Node *expr = parser.expectExpression(lexer.tokens().constBegin(), error);
	ast::Printer printer;
	printer.printToFile("ast.txt");
	if (expr) {
		printer.printNode(expr);
	}
}
