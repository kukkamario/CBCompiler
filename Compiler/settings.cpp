#include "settings.h"
#include <fstream>


namespace po = boost::program_options;


Settings::Settings() :
	mFVD(false),
	mConfigOptions("Basic options"),
	mCmdOptions("Generic options"),
	mConfigurationOptions("Configuration options") {


	mOpt = "opt %1% %2% -o %3%";
	mLLC = "llc %1% %2% -o %3%";
	mLinker = "g++ %1% %2% -o %3%";
	mOutputFile = "cbrun";
	mFunctionMapping = "runtime/functionmapping.map";
	mRuntimeLibrary = "runtime/libRuntime.bc";
	mDataTypes = "runtime/datatypes.json";



	mCmdOptions.add_options()
			("version,v", "Print version string")
			("help,h", "Produce help message");

	mConfigurationOptions.add_options()
			("opt_run", po::value<std::string>(), "opt run command")
			("opt_flags", po::value<std::vector<std::string>>(), "Flags given to opt")
			("llc_run", po::value<std::string>(), "llc run command")
			("llc_flags", po::value<std::vector<std::string>>(), "Flags given to llc")
			("linker_run", po::value<std::string>(), "Linker run command")
			("linker_flags", po::value<std::vector<std::string>>(), "Flags given to the linker")
			("runtime_library", po::value<std::string>(), "Path of the runtime library bitcode file")
			("function_mapping", po::value<std::string>(), "Function mapping file path")
			("force_variable_declaration,fvd", po::value<bool>(), "Force variable declaration")
			("output_file,o", po::value<std::string>(), "Output file")
			("data_types", po::value<std::string>(), "Custom data type configuration json-file path")
			;

}

bool Settings::parseCommandLine(int argc, char *argv[]) {
#ifdef _WIN32
	mLoadPath = "settings_win.ini";
#else
	mLoadPath = "settings_linux.ini";
#endif

	po::positional_options_description inputFileOptionDesc;
	inputFileOptionDesc.add("input_file", -1);

	po::options_description cmdOptions("");

	cmdOptions.add(mCmdOptions);
	cmdOptions.add(mConfigurationOptions);

	po::options_description configOptions("");
	configOptions.add(mConfigOptions);
	configOptions.add(mConfigurationOptions);

	po::variables_map varMap;
	try {
		po::store(po::command_line_parser(argc, argv).options(cmdOptions).positional(inputFileOptionDesc).run(), varMap);
		po::store(po::parse_config_file(mLoadPath.c_str(), configOptions), varMap);
	}
	catch (const std::exception &e) {
		std::cerr << e;
		return false;
	}

	po::notify(varMap);
	if (varMap.count("help")) {
		cmdOptions.print(std::cout);
		return false;
	}
	if (varMap.count("version")) {
		std::cout << "Version  0.1.0\n";
	}
	if (varMap.count("output_file")) {
		mOutputFile = varMap["output_file"].as<std::string>();
	}
	if (varMap.count("input_file")) {
		mInputFiles = varMap["input_file"].as<std::vector<std::string>>();
	}
	if (varMap.count("force_variable_declaration")) {
		mFVD = varMap["force_variable_declaration"].as<bool>();
	}
	if (varMap.count("runtime_library")) {
		mRuntimeLibrary = varMap["runtime_library"].as<std::string>();
	}
	if (varMap.count("function_mapping")) {
		mFunctionMapping = varMap["function_mapping"].as<std::string>();
	}

	if (varMap.count("opt_run")) {
		mOpt = varMap["opt_run"].as<std::string>();
	}
	if (varMap.count("opt_flags")) {
		mOpt = varMap["opt_flags"].as<std::string>();
	}

	if (varMap.count("llc_run")) {
		mLLC = varMap["llc_run"].as<std::string>();
	}
	if (varMap.count("llc_flags")) {
		mLLCFlags = varMap["llc_flags"].as<std::string>();
	}

	if (varMap.count("linker_run")) {
		mLinker = varMap["linker_run"].as<std::string>();
	}
	if (varMap.count("linker_flags")) {
		mLinkerFlags = varMap["linker_flags"].as<std::string>();
	}

	if (varMap.count("data_types")) {
		mDataTypes = varMap["data_types"].as<std::string>();
	}
	return true;
}

bool Settings::loadDefaults() {


	return true;
}

bool Settings::callOpt(const std::string &inputFile, const std::string &outputFile) const {
	std::string cmd = mOpt.arg(mOptFlags, inputFile, outputFile);
	qDebug() << cmd;
	qDebug() << QDir::currentPath();
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}

bool Settings::callLLC(const std::string &inputFile, const std::string &outputFile) const {
	std::string cmd = mLLC.arg(mLLCFlags, inputFile, outputFile);
	qDebug() << cmd;
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}

bool Settings::callLinker(const std::string &inputFile, const std::string &outputFile) const {
	std::string cmd = mLinker.arg(mLinkerFlags, inputFile, "\"" + outputFile + "\"");
	qDebug() << cmd;
	int ret = QProcess::execute(cmd);
	qDebug() << ret;
	return ret == 0;
}
