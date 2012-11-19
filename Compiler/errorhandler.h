#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
#include <QString>
#include <QObject>
class QFile;
class ErrorHandler : public QObject{
		Q_OBJECT
	public:
		ErrorHandler();
	public slots:
		void warning(int code, QString msg, int line, QFile *file);
		void error(int code, QString msg, int line, QFile *file);
};

#endif // ERRORHANDLER_H
