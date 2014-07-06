#ifndef LISTSTRINGJOIN_H
#define LISTJOIN_H
#include <QString>
#include <QList>
#include <QStringBuilder>
#include <QStringList>

template <typename T, typename FuncTy>
QString listStringJoin(const QList<T> &list, FuncTy func) {
	QStringList stringList;
	stringList.reserve(list.size());
	for (const T &v : list) {
		stringList.append(func(v));
	}
	return stringList.join(QString(", "));
}


#endif // LISTSTRINGJOIN_H
