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
#include <QSettings>

int main(int argc, char *argv[]) {
	QCoreApplication a(argc, argv);

	QTime bigTimer;
	bigTimer.start();

	ErrorHandler errHandler;


	bool success = false;
	success = Settings::loadDefaults();
	if (!success) {
		errHandler.error(ErrorCodes::ecSettingsLoadingFailed, errHandler.tr("Loading the default settings \"%1\" failed").arg(Settings::loadPath()), CodePoint());
		return ErrorCodes::ecSettingsLoadingFailed;
	}

	Settings::parseCommandLine(argc, argv);


	Lexer lexer;
	QObject::connect(&lexer, &Lexer::error, &errHandler, &ErrorHandler::error);
	QObject::connect(&lexer, &Lexer::warning, &errHandler, &ErrorHandler::warning);
	QTime timer;
	timer.start();
	if (lexer.tokenizeFile(Settings::inputFile()) == Lexer::Success) {
		qDebug() << "Lexical analysing took " << timer.elapsed() << "ms";
#ifdef DEBUG_OUTPUT
		lexer.writeTokensToFile("tokens.txt");
#endif
	}
	else {
		errHandler.error(ErrorCodes::ecLexicalAnalysingFailed, errHandler.tr("Lexical analysing failed"), CodePoint(lexer.files().first().first));
		return ErrorCodes::ecLexicalAnalysingFailed;
	}

	Parser parser;
	QObject::connect(&parser, &Parser::error, &errHandler, &ErrorHandler::error);
	QObject::connect(&parser, &Parser::warning, &errHandler, &ErrorHandler::warning);

	timer.start();
	ast::Program *program = parser.parse(lexer.tokens());
	qDebug() << "Parsing took " << timer.elapsed() << "ms";
	if (!parser.success()) {
		errHandler.error(ErrorCodes::ecParsingFailed, errHandler.tr("Parsing failed \"%1\"").arg(Settings::inputFile()), CodePoint());
		return ErrorCodes::ecParsingFailed;
	}

#ifdef DEBUG_OUTPUT
	if (program) {
		QFile file("ast.txt");
		if (file.open(QFile::WriteOnly)) {
			QTextStream stream(&file);
			program->write(stream);
			file.close();
		}
	}
#endif



	CodeGenerator codeGenerator;
	//QObject::connect(&codeGenerator, &CodeGenerator::error, &errHandler, &ErrorHandler::error);
	//QObject::connect(&codeGenerator, &CodeGenerator::warning, &errHandler, &ErrorHandler::warning);
	QObject::connect(&codeGenerator, &CodeGenerator::error, &errHandler, &ErrorHandler::error);
	QObject::connect(&codeGenerator, &CodeGenerator::warning, &errHandler, &ErrorHandler::warning);

	timer.start();
	if (!codeGenerator.initialize()) {
		return ErrorCodes::ecCodeGeneratorInitializationFailed;
	}


	qDebug() << "Code generator initialization took " << timer.elapsed() << "ms";
	timer.start();
	if (!codeGenerator.generate(program)) {
		errHandler.error(ErrorCodes::ecCodeGenerationFailed, errHandler.tr("Code generation failed"), CodePoint());
		return ErrorCodes::ecCodeGenerationFailed;
	}

	qDebug() << "Code generation took" << timer.elapsed() << "ms";
	qDebug() << "LLVM-IR generated";

	timer.start();
	codeGenerator.createExecutable(Settings::outputFile());
	qDebug() << "Executable generation took " << timer.elapsed() << "ms";
	qDebug() << "The whole compilation took " << bigTimer.elapsed() << "ms";
	return 0;
}
