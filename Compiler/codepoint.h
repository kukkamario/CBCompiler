#ifndef CODEPOINT_H
#define CODEPOINT_H
#include <QtGlobal>
#include <QString>
#include <QObject>
#include <QStringBuilder>

class CodePoint {
	public:
		CodePoint() : mLine(0), mColumn(0) { }
		CodePoint(const QString & file) : mLine(0), mColumn(0), mFile(file) {}
		CodePoint(int line, int column, QString file) : mLine(line), mColumn(column), mFile(file) {}
		bool isNull() const { return mFile.isNull() && mLine == 0 && mColumn == 0; }
		int line() const { return mLine; }
		int column() const { return mColumn; }
		QString file() const { return mFile; }

		QString toString() const;
	private:
		int mLine;
		int mColumn;
		QString mFile;
};

Q_DECLARE_METATYPE(CodePoint)


inline QString CodePoint::toString() const {
	if (!mFile.isEmpty()) {
		if (mLine && mColumn) {
			return QStringLiteral("\"") % mFile % "\" [" % QString::number(mLine) % ", " % QString::number(mColumn) % "]";
		}
		else {
			return QStringLiteral("\"") % mFile % "\"";
		}
	}
	return QString();
}

#endif // CODEPOINT_H
