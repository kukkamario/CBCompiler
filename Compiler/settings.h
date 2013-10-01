#ifndef SETTINGS_H
#define SETTINGS_H
#include <QString>
class Settings {
	public:
		Settings();
		bool loadDefaults();
		bool callOpt(const QString &inputFile, const QString &outputFile) const;
		bool callLLC(const QString &inputFile, const QString &outputFile) const;
		bool callLinker(const QString &inputFile, const QString &outputFile) const;
		bool forceVariableDeclaration() const { return mFVD; }
		QString defaultOutputFile() const { return mDefaultOutput; }
		QString loadPath() const { return mLoadPath; }
		QString runtimeLibraryPath() const { return mRuntimeLibrary; }
		QString dataTypesFile() const { return mDataTypes; }
		QString functionMappingFile() const { return mFunctionMapping; }
	private:
		QString mLoadPath;

		QString mOpt;
		QString mOptFlags;
		QString mLLC;
		QString mLLCFlags;
		QString mLinker;
		QString mLinkerFlags;

		bool mFVD;
		QString mDefaultOutput;
		QString mRuntimeLibrary;
		QString mFunctionMapping;
		QString mDataTypes;
};

#endif // SETTINGS_H
