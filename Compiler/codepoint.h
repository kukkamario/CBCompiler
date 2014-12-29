#ifndef CODEPOINT_H
#define CODEPOINT_H
#include <string>
#include <boost/utility/string_ref.hpp>
#include <boost/lexical_cast.hpp>

class CodePoint {
	public:
		CodePoint() : mLine(0), mColumn(0) { }
		CodePoint(boost::string_ref file) : mLine(0), mColumn(0), mFile(file) {}
		CodePoint(int line, int column, boost::string_ref file) : mLine(line), mColumn(column), mFile(file) {}
		bool isNull() const { return mFile.empty() && mLine == 0 && mColumn == 0; }
		int line() const { return mLine; }
		int column() const { return mColumn; }
		boost::string_ref file() const { return mFile; }

		std::string toString() const;
	private:
		int mLine;
		int mColumn;
		boost::string_ref mFile;
};


inline std::string CodePoint::toString() const {
	if (!mFile.empty()) {
		if (mLine && mColumn) {
			return std::string("\"") + mFile.to_string() + "\" [" + boost::lexical_cast<std::string>(mLine) + ", " + boost::lexical_cast<std::string>(mColumn) + "]";
		}
		else {
			return std::string("\"") + mFile.to_string() + "\"";
		}
	}
	return std::string();
}

#endif // CODEPOINT_H
