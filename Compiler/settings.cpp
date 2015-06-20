#include "settings.h"
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <llvm/Support/CommandLine.h>
#include <iostream>

static llvm::cl::opt<bool> sForceVariableDeclaration("fvd", llvm::cl::desc("Force variable declaration using Dim"));
static llvm::cl::opt<std::string> sInputFile(llvm::cl::Positional, llvm::cl::desc("<input file>"), llvm::cl::Required);
static llvm::cl::opt<std::string> sOutputFile("o", llvm::cl::desc("Specify output filename"), llvm::cl::value_desc("filename"));
static llvm::cl::opt<std::string> sRuntimeLibraryPath("rlpath", llvm::cl::desc("Specify runtime library path"));
static llvm::cl::opt<std::string> sFunctionMappingFile("fmappath", llvm::cl::desc("Function mapping filepath"));
static llvm::cl::opt<std::string> sDataTypesFile("datatypespath", llvm::cl::desc("Datatypes filepath"));
static QString sLoadPath;
static QString sOpt;
static QString sOptFlags;
static QString sLLC;
static QString sLLCFlags;
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

	var = settings.value("opt/call");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sOpt = var.toString();

	var = settings.value("opt/default-flags");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sOptFlags = var.toString();

	var = settings.value("llc/call");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sLLC = var.toString();

	var = settings.value("llc/default-flags");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	sLLCFlags = var.toString();

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

bool Settings::callOpt(const QString &inputFile, const QString &outputFile) {
	QString cmd = sOpt.arg(sOptFlags, inputFile, outputFile);
	qDebug() << cmd;
	qDebug() << QDir::currentPath();
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}

bool Settings::callLLC(const QString &inputFile, const QString &outputFile) {
	QString cmd = sLLC.arg(sLLCFlags, inputFile, outputFile);
	qDebug() << cmd;
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}

bool Settings::callLinker(const QString &inputFile, const QString &outputFile) {
	QString cmd = sLinker.arg(sLinkerFlags, inputFile, "\"" + outputFile + "\"");
	qDebug() << cmd;
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}

void Settings::parseCommandLine(int argc, char *argv[]) {
	llvm::cl::SetVersionPrinter([](){
		std::cout << "CBCompiler version 0.2.0\n";
	});
	llvm::cl::ParseCommandLineOptions(argc, argv);
}


bool Settings::forceVariableDeclaration() {
	return sForceVariableDeclaration;
}


QString Settings::outputFile() {
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
