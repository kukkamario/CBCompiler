#ifndef EXPRESSIONGENERATOR_H
#define EXPRESSIONGENERATOR_H
#include <QObject>
#include "valuetype.h"
#include "value.h"
#include "abstractsyntaxtree.h"
class QFile;
class ExpressionGenerator : public QObject {
	Q_OBJECT
	public:
		ExpressionGenerator();
		Value generate(ast::Node *);
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // EXPRESSIONGENERATOR_H
