
#include "lexer.h"
#include <QDebug>
#include <chrono>
#include "errorhandler.h"
#include "errorcodes.h"
#include "parser.h"
#include <iostream>
#include <boost/format.hpp>
#include "codegenerator.h"

using timer = std::chrono::steady_clock;

int main(int argc, char *argv[]) {


	timer::time_point startTime = timer::now();

	ErrorHandler errHandler;

	Settings settings;
	if (!settings.parseCommandLine(argc, argv)) {
		return 0;
	}

	if (settings.inputFiles().size() != 1) {
		errHandler.error(ErrorCodes::ecNoInputFiles, "Expecting one input file", CodePoint());
		return 0;
	}


	Lexer lexer(&errHandler);
	timer::time_point lexerStart = timer::now();
	if (lexer.tokenizeFile(settings.inputFiles().front(), &settings) == Lexer::Success) {
		std::cout << "Lexical analysing took " << std::chrono::duration_cast<std::chrono::milliseconds>(timer::now() - lexerStart).count() << "ms";
#ifdef DEBUG_OUTPUT
		lexer.writeTokensToFile("tokens.txt");
#endif
	}
	else {
		errHandler.error(ErrorCodes::ecLexicalAnalysingFailed, "Lexical analysing failed", CodePoint(lexer.files().front().first));
		return ErrorCodes::ecLexicalAnalysingFailed;
	}

	Parser parser(&errHandler);

	timer::time_point parserStart = timer::now();
	ast::Program *program = parser.parse(lexer.tokens(), &settings);
	std::cout << "Parsing took " << std::chrono::duration_cast<std::chrono::milliseconds>(timer::now() - parserStart).count() << "ms";
	if (!parser.success()) {
		errHandler.error(ErrorCodes::ecParsingFailed, "Parsing failed", CodePoint());
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



	CodeGenerator codeGenerator(&errHandler);
	timer::time_point generatorStart = timer::now();
	if (!codeGenerator.initialize(&settings)) {
		return ErrorCodes::ecCodeGeneratorInitializationFailed;
	}


	std::cout << "Code generator initialization took " << std::chrono::duration_cast<std::chrono::milliseconds>(timer::now() - generatorStart).count() << "ms";
	generatorStart = timer::now();
	if (!codeGenerator.generate(program)) {
		errHandler.error(ErrorCodes::ecCodeGenerationFailed, "Code generation failed", CodePoint());
		return ErrorCodes::ecCodeGenerationFailed;
	}

	std::cout << "Code generation took" << std::chrono::duration_cast<std::chrono::milliseconds>(timer::now() - generatorStart).count() << "ms";
	std::cout << "LLVM-IR generated";

	generatorStart = timer::now();
	codeGenerator.createExecutable(settings.outputFile());
	std::cout << "Executable generation took " << std::chrono::duration_cast<std::chrono::milliseconds>(timer::now() - generatorStart).count() << "ms";
	std::cout << "The whole compilation took " << std::chrono::duration_cast<std::chrono::milliseconds>(timer::now() - startTime).count() << "ms";
	return 0;
}
