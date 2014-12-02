#include "settings.h"
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>

Settings::Settings() :
	mFVD(false) {
}

bool Settings::loadDefaults() {


#ifdef _WIN32
	mLoadPath = QCoreApplication::applicationDirPath() + "\\settings_win_default.ini";
#else
	mLoadPath = QCoreApplication::applicationDirPath() + "/settings_linux_default.ini";
#endif


	QSettings settings(mLoadPath, QSettings::IniFormat);
	QVariant var = settings.value("compiler/force-variable-declaration");
	if (var.isNull() || !var.canConvert(QMetaType::Bool)) return false;
	mFVD = var.toBool();

	var = settings.value("compiler/default-output-file");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mDefaultOutput = var.toString();

	var = settings.value("compiler/runtime-library");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mRuntimeLibrary = var.toString();

	var = settings.value("compiler/function-mapping");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mFunctionMapping = var.toString();

	var = settings.value("compiler/data-types");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mDataTypes= var.toString();

	var = settings.value("opt/call");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mOpt = var.toString();

	var = settings.value("opt/default-flags");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mOptFlags = var.toString();

	var = settings.value("llc/call");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mLLC = var.toString();

	var = settings.value("llc/default-flags");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mLLCFlags = var.toString();

	var = settings.value("linker/call");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mLinker = var.toString();

	var = settings.value("linker/default-flags");
	if (var.isNull() ||  !var.canConvert(QMetaType::QString)) return false;
	mLinkerFlags = var.toString();

	QFileInfo fi;
	fi.setFile(QDir(QCoreApplication::applicationDirPath()), mDataTypes);
	mDataTypes = fi.absoluteFilePath();

	fi.setFile(QDir(QCoreApplication::applicationDirPath()), mRuntimeLibrary);
	mRuntimeLibrary = fi.absoluteFilePath();

	fi.setFile(QDir(QCoreApplication::applicationDirPath()), mFunctionMapping);
	mFunctionMapping = fi.absoluteFilePath();
	return true;
}

bool Settings::callOpt(const QString &inputFile, const QString &outputFile) const {
	QString cmd = mOpt.arg(mOptFlags, inputFile, outputFile);
	qDebug() << cmd;
	qDebug() << QDir::currentPath();
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}

bool Settings::callLLC(const QString &inputFile, const QString &outputFile) const {
	QString cmd = mLLC.arg(mLLCFlags, inputFile, outputFile);
	qDebug() << cmd;
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}

bool Settings::callLinker(const QString &inputFile, const QString &outputFile) const {
	QString cmd = mLinker.arg(mLinkerFlags, inputFile, "\"" + outputFile + "\"");
	qDebug() << cmd;
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}
