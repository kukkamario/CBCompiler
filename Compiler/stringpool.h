#ifndef STRINGPOOL_H
#define STRINGPOOL_H
#include <QString>
#include <QLinkedList>
#include <QMap>
class StringPool {
	public:
		StringPool();
		QString *add(const QString &s);
		void remove(QString *s);
	private:
		QLinkedList<QString> mStringList;
		QMap<QString*, QLinkedList<QString>::Iterator> mReverse;
};

#endif // STRINGPOOL_H
