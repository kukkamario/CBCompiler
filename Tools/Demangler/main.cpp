#include <iostream>
#include <fstream>
#include <cstring>
#include <llvm/Module.h>
#if LLVM_VERSION_MINOR < 3
	#include <llvm/LLVMContext.h>
	#include <llvm/Support/IRReader.h>
	#include <llvm/Function.h>
#else
	#include <llvm/IR/LLVMContext.h>
	#include <llvm/IRReader/IRReader.h>
	#include <llvm/IR/Function.h>
	#include <llvm/Support/SourceMgr.h>
#endif

using namespace std;

void printUsage() {
	cout << "Usage:\n"
		 << "demangler [options] <input bitcode file>\n"
		 << "Options:\n"
		 << "\t-o <output file>   --- Sets output file\n"
		 << "\t-s <starts with>   --- Outputs only names starting with the given string\n"
		 << "\t-e <ends with>     --- Outputs only names ending with the given string\n"
		 /*<< "\t-t                 --- Don't ignore template functions\n"
		 << "\t-r                 --- Remove parameter lists\n"*/
		 << "\t-h                 --- Shows this information\n"
		 << "\t-p                 --- Remove '-s' prefix from the output function names\n";

}

bool endsWith (std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

bool startsWith (std::string const &fullString, std::string const &beginning) {
	if (fullString.length() >= beginning.length()) {
		return (0 == fullString.compare (0, beginning.length(), beginning));
	} else {
		return false;
	}
}


std::string demangle(const std::string &mangled) {
	if (mangled[0] == '_' && mangled[1] == 'Z' && isdigit(mangled[2])) {
		size_t size = mangled[2] - '0';
		size_t i = 3;
		while (i < mangled.length() && isdigit(mangled[i])) {
			size *= 10;
			size += mangled[i] - '0';
			i++;
		}

		std::string ret(size, 0);
		size_t nameStart = i;
		while (i < mangled.length() && (i - nameStart) < size) {
			ret[(i - nameStart)] = mangled[i];
			i++;
		}
		if ((i - nameStart) != size) {
			return std::string();
		}
		return ret;
	}
	return std::string();
}

int main(int argc, char *argv[]) {
	string input;
	string output;
	string onlyStartingWith;
	string onlyEndingWith;
	bool removePrefix = false;
	if (argc == 1) {
		printUsage();
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		if (*argv[i] == '-') { //option
			if (argv[i][1] == 0) {
				cerr << "Error: Invalid option '-'\n";
				return 1;
			}

			switch (argv[i][1]) {
				case 'o':
					if (i + 1 < argc) {
						if (!output.empty()) {
							cerr << "Error: Output file is defined multiple times\n";
							return 1;
						}
						output = argv[++i];
						continue;
					}
					else {
						cerr << "Error: Expecting a parameter after '-o'\n";
						return 1;
					}
				case 's':
					if (i + 1 < argc) {
						if (!onlyStartingWith.empty()) {
							cerr << "Error: -s flag is defined multiple times\n";
							return 1;
						}
						onlyStartingWith = argv[++i];
						continue;
					}
					else {
						cerr << "Error: Expecting a parameter after '-e'\n";
						return 1;
					}
				case 'e':
					if (i + 1 < argc) {
						if (!onlyEndingWith.empty()) {
							cerr << "Error: -e is defined multiple times\n";
							return 1;
						}
						onlyEndingWith = argv[++i];
						continue;
					}
					else {
						cerr << "Error: Expecting a parameter after '-e'\n";
						return 1;
					}
				case 'h':
					printUsage();
					return 0;
				case 'p':
					removePrefix = true;
					continue;
				/*case 'c':
					exportClasses = true;
					continue; */
				default:
					cerr << "Error: Invalid parameter '" << argv[i] << "'\n";
					return 1;
			}
		} else {
			if (!input.empty()) {
				cerr << "Error: Input file already defined";
				return 1;
			}
			input = argv[i];
		}
	}

	if (input.empty()) {
		cerr << "Error: Input file is not specified\n";
		return 1;
	}
	else {
		cout << "Input file: " << input << endl;
	}

	if (output.empty()) {
		output = input + ".out";
	}
	cout << "Output file: " << output << endl;

	if (!onlyStartingWith.empty()) {
		cout << "Outputing only names starting with " << onlyStartingWith << endl;
	}

	if (!onlyEndingWith.empty()) {
		cout << "Outputing only names ending with " << onlyEndingWith << endl;
	}

	llvm::SMDiagnostic diagnostic;
	llvm::Module *module = llvm::ParseIRFile(input, diagnostic, llvm::getGlobalContext());
	if (!module) {
		cerr << diagnostic.getMessage() << endl;
		return 2;
	}

	ofstream out;
	out.open(output.c_str());
	if (!out.is_open()) {
		cerr << "Error: Can't open output file" << endl;
		return 1;
	}

	size_t len = 1024;
	char *buffer = (char*)malloc(len);
	string funcName;
	string mangledName;
	cout << "Starting exporting" << endl;
	for (llvm::Module::FunctionListType::iterator i = module->getFunctionList().begin(); i != module->getFunctionList().end(); i++) {

		mangledName = i->getName();
		/*
		int status = 0;
		abi::__cxa_demangle(mangledName.c_str(), buffer, &len, &status);
		if (status != 0) {
			if (status == -2) {
				funcName = i->getName().data();
			}
			else {
				cerr << "abi::__cxa_demangle failed with the status " <<  status << endl;
				cerr << "Function name: " << mangledName << endl;
				return status;
			}
		}
		else {
			funcName = buffer;
		}*/
		funcName = demangle(mangledName);
		if (funcName.empty()) continue;

		if (!onlyStartingWith.empty()) {
			if (!startsWith(funcName, onlyStartingWith)) continue;
			if (removePrefix) {
				funcName.erase(0, onlyStartingWith.length());
			}
		}
		if (!onlyEndingWith.empty()) {
			if (!endsWith(funcName, onlyEndingWith)) continue;
		}
		/*if (ignoreTemplates) {
			if (funcName.find('<') != string::npos) continue;
		}
		if (removeParameters) {
			string::size_type paramListBegin = funcName.find('(');
			if (paramListBegin != string::npos) {
				funcName.erase(paramListBegin);
			}
		}*/
		cout << mangledName << "   ----->   " << funcName << endl;

		out << funcName << '=' << mangledName << '\n';
	}
	free(buffer);
	out.close();
	cout << "Exporting succeeded" << endl;
	return 0;
}



