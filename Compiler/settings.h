#ifndef SETTINGS_H
#define SETTINGS_H
#include <QString>
namespace Settings {
	bool loadDefaults();
	void parseCommandLine(int argc, char *argv[]);
	bool callOpt(const QString &inputFile, const QString &outputFile);
	bool callLLC(const QString &inputFile, const QString &outputFile);
	bool callLinker(const QString &inputFile, const QString &outputFile);
	bool forceVariableDeclaration();
	QString inputFile();
	QString outputFile();
	QString loadPath();
	QString runtimeLibraryPath();
	QString dataTypesFile();
	QString functionMappingFile();

}

#endif // SETTINGS_H
