#include "codegenerator.h"

CodeGenerator::CodeGenerator()
{
}

bool CodeGenerator::initialize(const QString &runtimeFile) {
	if (!mRuntime.load(runtimeFile)) {
		return false;
	}
	return true;
}

bool CodeGenerator::generate(ast::Program *program) {

	return true;
}
