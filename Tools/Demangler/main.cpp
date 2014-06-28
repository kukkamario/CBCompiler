#include <QCoreApplication>
#include <QString>
#include <QProcess>
#include <QTextStream>
#include <QStringList>
#include <QByteArray>
#include <QList>
#include <iostream>
#include <fstream>
#include <cstring>
#include <QFile>
#include <QDebug>
#include <llvm/Config/llvm-config.h>
#if LLVM_VERSION_MINOR < 3
	#include <llvm/LLVMContext.h>
	#include <llvm/Support/IRReader.h>
	#include <llvm/Function.h>
	#include <llvm/Module.h>
#else
	#include <llvm/IR/Module.h>
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


int main(int argc, char *argv[]) {

	QString input;
	QString output;
	QByteArray onlyStartingWith;
	QByteArray onlyEndingWith;
	bool removePrefix = false;
	QCoreApplication app(argc, argv);
	if (argc == 1) {
		printUsage();
		return 0;
	}
	QStringList args = app.arguments();
	for (QStringList::Iterator i = ++args.begin(); i != args.end(); i++) {
		QString arg = *i;
		if (arg.startsWith('-')) { //option
			if (arg.length() == 1) {
				cerr << "Error: Invalid option '-'\n";
				return 1;
			}

			if (arg.length() == 2) {
				switch (arg[1].toLatin1()) {
					case 'o':
						if (i + 1 != args.end()) {
							if (!output.isEmpty()) {
								cerr << "Error: Output file is defined multiple times\n";
								return 1;
							}
							output = *(++i);
							continue;
						}
						else {
							cerr << "Error: Expecting a parameter after '-o'\n";
							return 1;
						}
					case 's':
						if (i + 1 != args.end()) {
							if (!onlyStartingWith.isEmpty()) {
								cerr << "Error: -s flag is defined multiple times\n";
								return 1;
							}
							onlyStartingWith = (*(++i)).toLatin1();
							continue;
						}
						else {
							cerr << "Error: Expecting a parameter after '-s'\n";
							return 1;
						}
					case 'e':
						if (i + 1 != args.end()) {
							if (!onlyEndingWith.isEmpty()) {
								cerr << "Error: -e is defined multiple times\n";
								return 1;
							}
							onlyEndingWith = (*(++i)).toLatin1();
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
				}
			}
			cerr << "Error: Invalid parameter '" << qPrintable(arg) << "'\n";
			return 1;
		} else {
			if (!input.isEmpty()) {
				cerr << "Error: Input file already defined";
				return 1;
			}
			input = arg;
		}
	}

	if (input.isEmpty()) {
		cerr << "Error: Input file is not specified\n";
		return 1;
	}
	else {
		cout << "Input file: " << qPrintable(input) << endl;
	}

	if (output.isEmpty()) {
		output = input + ".out";
	}
	cout << "Output file: " << qPrintable(output) << endl;

	if (!onlyStartingWith.isEmpty()) {
		cout << "Outputting only names starting with " << qPrintable(onlyStartingWith) << endl;
	}

	if (!onlyEndingWith.isEmpty()) {
		cout << "Outputting only names ending with " << qPrintable(onlyEndingWith) << endl;
	}

	llvm::SMDiagnostic diagnostic;
	llvm::Module *module = llvm::ParseIRFile(input.toStdString(), diagnostic, llvm::getGlobalContext());
	if (!module) {
		cerr << diagnostic.getMessage().data() << endl;
		return 2;
	}

	cout << "Starting exporting" << endl;

	QList<QByteArray> mangledNames;
	for (llvm::Module::FunctionListType::iterator i = module->getFunctionList().begin(); i != module->getFunctionList().end(); i++) {
		QByteArray n = QByteArray(i->getName().data());
		mangledNames.append(n);
	}

	QProcess process;
	process.start("c++filt -s gnu-v3");
	if (!process.waitForStarted()) {
		cerr << "Can't start c++filt";
		return 0;
	}
	cout << "c++filt started" << endl;
	for (QByteArray s : mangledNames) {
		if (s.startsWith('_')) {
			process.write('_' + s + '\n');
		}
		else {
			process.write(s + '\n');
		}
	}
	cout << "Waiting for writing" << endl;
	process.waitForBytesWritten();
	process.closeWriteChannel();
	cout << "Waiting for reading" << endl;
	process.waitForReadyRead();
	process.waitForFinished();
	QByteArray s = process.readAll();
	QFile file(output);
	if (!file.open(QFile::WriteOnly)) {
		cerr << "Can't open output file " << qPrintable(output) << endl;
		return -1;
	}

	QList<QByteArray> demangledNames = s.split('\n');
	qDebug() << "Demangled items: "  << demangledNames.size();
	int index = -1;
	for (QByteArray name : demangledNames) {
		index++;
		qDebug() << name;
		if (!onlyStartingWith.isEmpty()) {
			if (!name.startsWith(onlyStartingWith)) {
				continue;
			} else if (removePrefix) {
				name = name.mid(onlyStartingWith.length());
			}
		}

		int parameterListStart = name.indexOf('(');
		int templateParamListStart = name.indexOf('<');
		if (parameterListStart == -1) {
			cerr << "Invalid mangled name: " << name.data() << endl;
			continue;
		}
		if (templateParamListStart != -1 && templateParamListStart < parameterListStart)
			name = name.left(templateParamListStart);
		else
			name = name.left(parameterListStart).trimmed();

		if (!onlyEndingWith.isEmpty() && !name.endsWith(onlyEndingWith)) {
			continue;
		}
		file.write(name  + "=" + mangledNames[index] + "\n");
	}
	file.close();

	cout << "Exporting succeeded" << endl;
	return 0;
}



