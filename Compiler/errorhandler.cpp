#include "errorhandler.h"
#include <iostream>
#include <QFile>
#include <QDebug>

ErrorHandler::ErrorHandler()
{
}

void ErrorHandler::warning(int code, QString msg, int line, QFile *file) {
	if (file) {
		if (line) {
			qCritical("\"%s\" [%i] Warning %i: %s",qPrintable(file->fileName()), line,  code, qPrintable(msg));
		}
		else {
			qCritical("\"%s\" Warning %i: %s",qPrintable(file->fileName()),  code, qPrintable(msg));
		}
	}
	else {
		qCritical("Warning %i: %s",  code, qPrintable(msg));
	}
}

void ErrorHandler::error(int code, QString msg, int line, QFile *file) {
	if (file) {
		if (line) {
			qCritical("\"%s\" [%i] Error %i: %s",qPrintable(file->fileName()), line,  code, qPrintable(msg));
		}
		else {
			qCritical("\"%s\" Error %i: %s",qPrintable(file->fileName()),  code, qPrintable(msg));
		}
	}
	else {
		qCritical("Error %i: %s",  code, qPrintable(msg));
	}
}
