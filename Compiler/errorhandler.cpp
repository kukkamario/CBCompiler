#include "errorhandler.h"
#include <iostream>
#include <QFile>
#include <QDebug>

ErrorHandler::ErrorHandler()
{
}

void ErrorHandler::warning(int code, QString msg, CodePoint cp) {
	qCritical("%s Warning %i: %s", qPrintable(cp.toString()), code, qPrintable(msg));
}

void ErrorHandler::error(int code, QString msg, CodePoint cp) {
	qCritical("%s Error %i: %s", qPrintable(cp.toString()), code, qPrintable(msg));
}
