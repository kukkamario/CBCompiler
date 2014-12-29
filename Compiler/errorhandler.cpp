#include "errorhandler.h"
#include <iostream>
#include <QFile>
#include <QDebug>

ErrorHandler::ErrorHandler() {
}

ErrorHandler::~ErrorHandler() {

}

void ErrorHandler::warning(int code, std::string msg, CodePoint cp) {
	std::cerr << cp.toString() << " Warning " << code << ": " << msg;
}

void ErrorHandler::error(int code, std::string msg, CodePoint cp) {
	std::cerr << cp.toString() << " Error " << code << ": " << msg;
}
