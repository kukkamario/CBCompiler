#ifndef SETTINGS_H
#define SETTINGS_H
#include <QString>
namespace Settings {
enum OptimizationLevel {
	O0,
	O1,
	O2,
	O3
};

enum OutputFileType {
	Executable,
	Library,
	Assembly,
	LLVM_IR,
	Null,
	ObjectFile
};

enum PrintLevel {
	Quiet,
	Normal,
	Verbose
};
	bool loadDefaults();
	bool parseCommandLine(int argc, char *argv[]);
	bool callLinker(const QString &inputFile, const QString &outputFile);
	bool forceVariableDeclaration();
	QString inputFile();
	QString outputFile();
	QString loadPath();
	QString runtimeLibraryPath();
	QString dataTypesFile();
	QString functionMappingFile();
	OptimizationLevel optLevel();
	PrintLevel printLevel();
	bool verbose();
	bool quiet();
	bool debugInfo();
	bool emitLLVMIR();
	QString saveAstFile();
	QString saveScopesFile();
	QString saveUnoptimizedBitcodeFile();
	QString saveTokensFile();

	OutputFileType outputFileType();
	bool compileOnly();

	QString outputExtension();

}

#endif // SETTINGS_H
