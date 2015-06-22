#include "settings.h"
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <llvm/Target/TargetMachine.h>
#include <string>

namespace cl = llvm::cl;
static cl::opt<bool> sForceVariableDeclaration("fvd", cl::desc("Force variable declaration using Dim"));
static cl::opt<std::string> sInputFile(cl::Positional, cl::desc("<input file>"), cl::Required);
static cl::opt<std::string> sOutputFile("o", cl::desc("Specify output filename"), cl::value_desc("filename"));
static cl::opt<std::string> sRuntimeLibraryPath("rlpath", cl::desc("Specify runtime library path"));
static cl::opt<std::string> sFunctionMappingFile("fmappath", cl::desc("Function mapping filepath"));
static cl::opt<std::string> sDataTypesFile("datatypespath", cl::desc("Datatypes filepath"));
static cl::opt<std::string> sSaveAstFile("ast", cl::desc("Save the abstract syntax tree to given file"));
static cl::opt<std::string> sSaveScopesFile("scopes", cl::desc("Save the scopes to given file"));
static cl::opt<std::string> sSaveTokensFile("tokens", cl::desc("Save tokens to given file"));
static cl::opt<std::string> sSaveUnoptimizedBitcodeFile("unoptllvm", cl::desc("Save llvm-ir before optimization passes to given file"));
cl::opt<Settings::OutputFileType>
sFileType("filetype", cl::init(Settings::Executable),
		 cl::desc("Choose a file type (not all types are supported by all targets):"),
		 cl::values(
			 clEnumValN(Settings::Assembly, "asm",
						"Emit an assembly ('.s') file"),
			 clEnumValN(Settings::ObjectFile, "obj",
						"Emit a native object ('.o') file"),
			 clEnumValN(Settings::Null, "null",
						"Emit nothing, for performance testing"),
			 clEnumValN(Settings::Executable, "exe",
						"Emit an executable file"),
			 //clEnumValN(Settings::Library, "lib", "Emit a library file),
			 clEnumValN(Settings::LLVM_IR, "bc", "Emit a llvm bitcode (.bc) file"),
			 clEnumValEnd));

extern cl::opt<char> OptLevel;
extern cl::opt<llvm::TargetMachine::CodeGenFileType> FileType;
static cl::opt<bool> sDebugInfo("g", cl::desc("Generate debug info"));
static cl::opt<bool> sQuietMode("quiet", cl::desc("Don't print informational messages"));
static cl::alias sQuietA("q", cl::desc("Alias for -quiet"), cl::aliasopt(sQuietMode));
static cl::opt<bool> sVerboseMode("verbose", cl::desc("Print as much information as possible. Overrides -quiet"));

static QString sLoadPath;
static QString sLinker;
static QString sLinkerFlags;


bool Settings::loadDefaults() {

#ifdef _WIN32
	sLoadPath = QCoreApplication::applicationDirPath() + "\\settings_win.ini";
#else
	sLoadPath = QCoreApplication::applicationDirPath() + "/settings_linux.ini";
#endif

	QSettings settings(sLoadPath, QSettings::IniFormat);
	QVariant var = settings.value("compiler/force-variable-declaration");
	if (var.isNull() || !var.canConvert(QMetaType::Bool)) return false;
	sForceVariableDeclaration.setInitialValue(var.toBool());

	var = settings.value("compiler/default-output-file");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sOutputFile.setInitialValue(var.toString().toStdString());

	var = settings.value("compiler/runtime-library");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sRuntimeLibraryPath.setInitialValue(var.toString().toStdString());

	var = settings.value("compiler/function-mapping");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sFunctionMappingFile.setInitialValue(var.toString().toStdString());

	var = settings.value("compiler/data-types");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sDataTypesFile.setInitialValue(var.toString().toStdString());

	var = settings.value("linker/call");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sLinker = var.toString();

	var = settings.value("linker/default-flags");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sLinkerFlags = var.toString();



	QFileInfo fi;
	fi.setFile(QDir(QCoreApplication::applicationDirPath()), QString::fromStdString(sDataTypesFile));
	sDataTypesFile.setInitialValue(fi.absoluteFilePath().toStdString());

	fi.setFile(QDir(QCoreApplication::applicationDirPath()), QString::fromStdString(sRuntimeLibraryPath));
	sRuntimeLibraryPath.setInitialValue(fi.absoluteFilePath().toStdString());

	fi.setFile(QDir(QCoreApplication::applicationDirPath()), QString::fromStdString(sFunctionMappingFile));
	sFunctionMappingFile.setInitialValue(fi.absoluteFilePath().toStdString());
	return true;
}

bool Settings::callLinker(const QString &inputFile, const QString &outputFile) {
	QString cmd = sLinker.arg(sLinkerFlags, inputFile, "\"" + outputFile + "\"");
	qDebug() << cmd;
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}

bool Settings::parseCommandLine(int argc, char *argv[]) {
	cl::ParseCommandLineOptions(argc, argv);

	switch (OptLevel) {
		default:
			std::cerr << argv[0] << ": invalid optimization level.\n";
			return false;
		case ' ': break;
		case '0': break;
		case '1': break;
		case '2': break;
		case '3': break;
	}

	return true;
}


bool Settings::forceVariableDeclaration() {
	return sForceVariableDeclaration;
}


QString Settings::outputFile() {
	std::string output(sOutputFile);
	if (output.empty()) {
		QFileInfo fi(inputFile());
		return fi.baseName() + outputExtension();
	}
	if (output.find('.') == std::string::npos){
		return QString::fromStdString(sOutputFile) + outputExtension();

	}
	return QString::fromStdString(sOutputFile);
}


QString Settings::loadPath() {
	return sLoadPath;
}


QString Settings::runtimeLibraryPath() {
	return QString::fromStdString(sRuntimeLibraryPath);
}


QString Settings::dataTypesFile() {
	return QString::fromStdString(sDataTypesFile);
}


QString Settings::functionMappingFile() {
	return QString::fromStdString(sFunctionMappingFile);
}


QString Settings::inputFile() {
	return QString::fromStdString(sInputFile);
}


Settings::OptimizationLevel Settings::optLevel() {
	switch (OptLevel) {
		default:
			assert(0 && "Invalid opt level");
		case ' ': return Settings::O2;
		case '0': return Settings::O0;
		case '1': return Settings::O1;
		case '2': return Settings::O2;
		case '3': return Settings::O3;
	}
}


Settings::PrintLevel Settings::printLevel() {
	if (sVerboseMode) return Settings::Verbose;
	if (sQuietMode) return Settings::Quiet;
	return Settings::Normal;
}


bool Settings::verbose() {
	return printLevel() == Settings::Verbose;
}


bool Settings::quiet() {
	return printLevel() == Settings::Quiet;
}


bool Settings::debugInfo() {
	return sDebugInfo;
}


QString Settings::saveTokensFile() {
	return QString::fromStdString(sSaveTokensFile);
}


QString Settings::saveUnoptimizedBitcodeFile() {
	return QString::fromStdString(sSaveUnoptimizedBitcodeFile);
}


QString Settings::saveScopesFile() {
	return QString::fromStdString(sSaveScopesFile);
}


QString Settings::saveAstFile() {
	return QString::fromStdString(sSaveAstFile);
}


bool Settings::compileOnly() {
	switch (outputFileType()) {
		case Executable:
		case Library:
			return false;
		case Assembly:
		case LLVM_IR:
		case Null:
		case ObjectFile:
			return true;
		default:
			assert(0 && "Invalid output file type");
			return true;

	}
}


bool Settings::emitLLVMIR() {
	return outputFileType() == LLVM_IR;
}


Settings::OutputFileType Settings::outputFileType() {
	return sFileType;
}


QString Settings::outputExtension() {
	switch (outputFileType()) {
		case Executable:
#ifdef Q_OS_WIN
			return ".exe";
#else
			return "";
#endif
		case Library:
			return ".a";
		case Assembly:
			return ".s";
		case LLVM_IR:
			return ".bc";
		case Null:
			return ".null";
		case ObjectFile:
			return ".o";
		default:
			assert(0 && "Invalid output file type");
			return QString();
	}
}
