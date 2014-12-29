#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
#include <string>
#include "codepoint.h"
class ErrorHandler {
	public:
		ErrorHandler();
		virtual ~ErrorHandler();
		virtual void warning(int code, std::string msg, CodePoint cp);
		virtual void error(int code, std::string msg, CodePoint cp);

};

#endif // ERRORHANDLER_H
