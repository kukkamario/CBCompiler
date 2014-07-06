#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
#include <QString>
#include <QObject>
#include "codepoint.h"
class QFile;
class ErrorHandler : public QObject{
		Q_OBJECT
	public:
		ErrorHandler();
	public slots:
		void warning(int code, QString msg, CodePoint cp);
		void error(int code, QString msg, CodePoint cp);
};

#endif // ERRORHANDLER_H
