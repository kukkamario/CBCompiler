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
			qCritical("Warning [%i] %s at line %i in file %s", code, qPrintable(msg), line, qPrintable(file->fileName()));
		}
		else {
			qCritical("Warning [%i] %s in file %s", code, qPrintable(msg), qPrintable(file->fileName()));
		}
	}
	else {
		qCritical("Warning [%i] %s", code, qPrintable(msg));
	}
}

void ErrorHandler::error(int code, QString msg, int line, QFile *file) {
	if (file) {
		if (line) {
			qCritical("Error [%i] %s at line %i in file %s", code, qPrintable(msg), line, qPrintable(file->fileName()));
		}
		else {
			qCritical("Error [%i] %s in file %s", code, qPrintable(msg), qPrintable(file->fileName()));
		}
	}
	else {
		qCritical("Error [%i] %s", code, qPrintable(msg));
	}
}
