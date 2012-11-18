#include "stringpool.h"

StringPool::StringPool() {
}

QString *StringPool::add(const QString &s) {
	mStringList.append(s);
	QString *sp = &mStringList.last();
	mReverse[sp] = --mStringList.end();
	return sp;
}

void StringPool::remove(QString *s) {
	QMap<QString*, QLinkedList<QString>::Iterator>::Iterator i = mReverse.find(s);
	Q_ASSERT(i != mReverse.end());
	mStringList.erase(i.value());
	mReverse.erase(i);
}
