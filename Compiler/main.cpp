#include <QCoreApplication>
#include <QStringList>
#include "lexer.h"
#include <QDebug>
#include <QTime>
#include "errorhandler.h"
#include "errorcodes.h"
#include "parser.h"
#include <iostream>
#include "codegenerator.h"
int main(int argc, char *argv[]) {
	QCoreApplication a(argc, argv);
	QStringList params = a.arguments();
	if (params.size() != 3) {
		qCritical() << "Expecting 2 arguments";
		return 0;
	}
	ErrorHandler errHandler;
	Lexer lexer;
	errHandler.connect(&lexer, SIGNAL(error(int,QString,int,QFile*)), SLOT(error(int,QString,int,QFile*)));
	errHandler.connect(&lexer, SIGNAL(warning(int,QString,int,QFile*)), SLOT(warning(int,QString,int,QFile*)));
	QTime startTime = QTime::currentTime();
	if (lexer.tokenizeFile(params[1]) == Lexer::Success) {
		qDebug() << "Lexical analysing took " << startTime.msecsTo(QTime::currentTime()) << "ms";
#ifdef _DEBUG
		lexer.writeTokensToFile("tokens.txt");
#endif
	}
	else {
		errHandler.error(ErrorCodes::ecLexicalAnalysingFailed, errHandler.tr("Lexical analysing failed"), 0, lexer.files().first().first);
		return 1;
	}

	Parser parser;
	errHandler.connect(&parser, SIGNAL(error(int,QString,int,QFile*)), SLOT(error(int,QString,int,QFile*)));
	errHandler.connect(&parser, SIGNAL(warning(int,QString,int,QFile*)), SLOT(warning(int,QString,int,QFile*)));

	startTime = QTime::currentTime();
	ast::Program *program = parser.parse(lexer.tokens());
	qDebug() << "Parsing took " << startTime.msecsTo(QTime::currentTime()) << "ms";
	ast::Printer printer;
	printer.printToFile("ast.txt");
	if (program) {
#ifdef _DEBUG
		printer.printProgram(program);
#endif
	}
	if (!parser.success()) {
		errHandler.error(ErrorCodes::ecParsingFailed, errHandler.tr("Parsing failed"), 0, lexer.files().first().first);
		return 1;
	}

	CodeGenerator codeGenerator;
	errHandler.connect(&codeGenerator, SIGNAL(error(int,QString,int,QFile*)), SLOT(error(int,QString,int,QFile*)));
	errHandler.connect(&codeGenerator, SIGNAL(warning(int,QString,int,QFile*)), SLOT(warning(int,QString,int,QFile*)));

	if (codeGenerator.initialize(params[2])) {
		codeGenerator.generate(program);
		return 0;
	}

	return 1;
}
