#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <boost/program_options.hpp>
class Settings {
	public:
		Settings();
		bool parseCommandLine(int argc, char *argv[]);
		bool callOpt(const std::string &inputFile, const std::string &outputFile) const;
		bool callLLC(const std::string &inputFile, const std::string &outputFile) const;
		bool callLinker(const std::string &inputFile, const std::string &outputFile) const;
		bool forceVariableDeclaration() const { return mFVD; }
		std::string loadPath() const { return mLoadPath; }
		std::string runtimeLibraryPath() const { return mRuntimeLibrary; }
		std::string dataTypesFile() const { return mDataTypes; }
		std::string functionMappingFile() const { return mFunctionMapping; }
		const std::vector<std::string> &inputFiles() const { return mInputFiles; }
		std::string outputFile() const { return mOutputFile; }
	private:
		std::string mLoadPath;

		std::string mOpt;
		std::vector<std::string> mOptFlags;
		std::string mLLC;
		std::string mLLCFlags;
		std::string mLinker;
		std::string mLinkerFlags;

		std::vector<std::string> mInputFiles;
		std::string mOutputFile;

		bool mFVD;
		std::string mRuntimeLibrary;
		std::string mFunctionMapping;
		std::string mDataTypes;

		boost::program_options::options_description mConfigOptions;
		boost::program_options::options_description mCmdOptions;
		boost::program_options::options_description mConfigurationOptions;
};

#endif // SETTINGS_H
