#ifndef SYSTEM_H
#define SYSTEM_H
#include <ctime>
class LString;
namespace sys {
	void init();
	double timeInSec();
	clock_t timeInMSec();
	void closeProgram();
	bool errorMessagesEnabled();
	void writeToOutput(const LString &s);
	void writeToError(const LString &s);
}
#endif // SYSTEM_H
