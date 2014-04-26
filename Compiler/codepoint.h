#ifndef CODEPOINT_H
#define CODEPOINT_H
#include <QtGlobal>
#include <QString>

class CodePoint {
	public:
		CodePoint() : mLine(0), mColumn(0) { }
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

Q_DECLARE_METATYPE(CodePoint);


QString CodePoint::toString() const {
	return '"' + mFile + "\" [" + mLine + ", " + mColumn + ']';
}

#endif // CODEPOINT_H
