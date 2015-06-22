#ifndef LLC_H
#define LLC_H
#include "llvm.h"

class LLC {
	public:
		LLC(int argc, char *Argv[]);

		bool compileModule(llvm::Module *module, std::string &outputFile);

	private:
		struct LLCImpl;
		LLCImpl *impl;
};

#endif // LLC_H
