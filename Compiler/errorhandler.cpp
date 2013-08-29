#include "errorhandler.h"
#include <iostream>
#include <QFile>
#include <QDebug>

ErrorHandler::ErrorHandler()
{
}

void ErrorHandler::warning(int code, QString msg, int line, QString file) {
	if (!file.isEmpty()) {
		if (line) {
			qCritical("\"%s\" [%i] Warning %i: %s",qPrintable(file), line,  code, qPrintable(msg));
		}
		else {
			qCritical("\"%s\" Warning %i: %s",qPrintable(file),  code, qPrintable(msg));
		}
	}
	else {
		qCritical("Warning %i: %s",  code, qPrintable(msg));
	}
}

void ErrorHandler::error(int code, QString msg, int line, QString file) {
	if (!file.isEmpty()) {
		if (line) {
			qCritical("\"%s\" [%i] Error %i: %s",qPrintable(file), line,  code, qPrintable(msg));
		}
		else {
			qCritical("\"%s\" Error %i: %s",qPrintable(file),  code, qPrintable(msg));
		}
	}
	else {
		qCritical("Error %i: %s",  code, qPrintable(msg));
	}
}
