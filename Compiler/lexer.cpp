#include "lexer.h"
#include "errorcodes.h"
#include <QTextStream>
#include <QDir>
#include <QDebug>
Lexer::Lexer()
{
	mKeywords["not"] = Token::opNot;
	mKeywords["or"] = Token::opOr;
	mKeywords["xor"] = Token::opXor;
	mKeywords["mod"] = Token::opMod;
	mKeywords["shr"] = Token::opShr;
	mKeywords["shl"] = Token::opShl;
	mKeywords["sar"] = Token::opSar;
	mKeywords["and"] = Token::opAnd;

	mKeywords["if"] = Token::kIf;
	mKeywords["then"] = Token::kThen;
	mKeywords["elseif"] = Token::kElseIf;
	mKeywords["else"] = Token::kElse;
	mKeywords["endif"] = Token::kEndIf;
	mKeywords["select"] = Token::kSelect;
	mKeywords["case"] = Token::kCase;
	mKeywords["default"] = Token::kDefault;
	mKeywords["endselect"] = Token::kEndSelect;
	mKeywords["function"] = Token::kFunction;
	mKeywords["return"] = Token::kReturn;
	mKeywords["endfunction"] = Token::kEndFunction;
	mKeywords["type"] = Token::kType;
	mKeywords["field"] = Token::kField;
	mKeywords["endtype"] = Token::kEndType;
	mKeywords["while"] = Token::kWhile;
	mKeywords["wend"] = Token::kWend;
	mKeywords["repeat"] = Token::kRepeat;
	mKeywords["forever"] = Token::kForever;
	mKeywords["until"] = Token::kUntil;
	mKeywords["goto"] = Token::kGoto;
	mKeywords["gosub"] = Token::kGosub;
	mKeywords["for"] = Token::kFor;
	mKeywords["to"] = Token::kTo;
	mKeywords["each"] = Token::kEach;
	mKeywords["step"] = Token::kStep;
	mKeywords["next"] = Token::kNext;
	mKeywords["dim"] = Token::kDim;
	mKeywords["redim"] = Token::kRedim;
	mKeywords["cleararray"] = Token::kClearArray;
	mKeywords["const"] = Token::kConst;
	mKeywords["global"] = Token::kGlobal;
	mKeywords["data"] = Token::kData;
	mKeywords["read"] = Token::kRead;
	mKeywords["restore"] = Token::kRestore;
	mKeywords["as"] = Token::kAs;
	mKeywords["exit"] = Token::kExit;
	mKeywords["end"] = Token::kEnd;

	mKeywords["new"] = Token::kNew;
	mKeywords["first"] = Token::kFirst;
	mKeywords["last"] = Token::kLast;
	mKeywords["before"] = Token::kBefore;
	mKeywords["after"] = Token::kAfter;

	mKeywords["include"] = Token::kInclude;
}

Lexer::ReturnState Lexer::tokenizeFile(const QString &file, const Settings &settings) {
	mSettings = settings;

	Lexer::ReturnState ret = tokenize(file);
	if (ret == Lexer::Error) return Lexer::Error;

	combineTokens();

	//Dirty trick, but works
	mTokens.append(Token(Token::EOL, mFiles.first().second.end(), mFiles.first().second.end(), 0, mFiles.first().first));
	mTokens.append(Token(Token::EOL, mFiles.first().second.end(), mFiles.first().second.end(), 0, mFiles.first().first));
	mTokens.append(Token(Token::EndOfTokens, mFiles.first().second.end(), mFiles.first().second.end(), 0, mFiles.first().first));
	return ret;
}

Lexer::ReturnState Lexer::tokenize(const QString &file) {
	QFile curFile(file);
	if (!curFile.open(QFile::ReadOnly | QFile::Text)) {
		mFiles.append(QPair<QString, QString>(curFile.fileName(), ""));
		emit error(ErrorCodes::ecCantOpenFile, tr("Cannot open file %1").arg(file), 0, "");
		return Error;
	}
	qDebug("File \"%s\" opened", qPrintable(file));
	QString oldPath = QDir::currentPath();
	QFileInfo fi(curFile);
	bool success = QDir::setCurrent(fi.absolutePath());
	assert(success);

	QString curFilePath = curFile.fileName();

	QTextStream stream(&curFile);
	QString code2 = stream.readAll();
	curFile.close();
	mFiles.append(QPair<QString, QString>(file, code2));
	QString &code = mFiles.last().second;

	ReturnState state = Success;
	int line = 1;
	for (QString::iterator i = code.begin(); i != code.end();) {
		if (i->category() == QChar::Separator_Space || *i == char(9) /* horizontal tab */) { // Space
			i++;
			continue;
		}
		if (*i == '\'') { //Single line comment
			i++;
			if (i == code.end()) return state;
			readToEOL(i, code.end());
			if (i != code.end()) {
				addToken(Token(Token::EOL, i, i + 1, line, curFilePath));
				++i;
				line++;
			}
			continue;
		}
		if (*i == '/') {
			i++;
			if (i != code.end()) {
				if (*i == '/') {
					i++;
					readToEOL(i, code.end());
					addToken(Token(Token::EOL, i, i + 1, line, curFilePath));
					i++;
					line++;
					continue;
				}
			}
			i--;
			addToken(Token(Token::opDivide, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '\n') {
			addToken(Token(Token::EOL, i, i + 1, line, curFilePath));
			i++;
			line++;
			continue;
		}
		if (*i == ',') {
			addToken(Token(Token::Comma, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == ':') {
			addToken(Token(Token::Colon, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '(') {
			addToken(Token(Token::LeftParenthese, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == ')') {
			addToken(Token(Token::RightParenthese, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '"') {
			readString(i, code.end(), line, curFilePath);
			continue;
		}
		if (*i == '*') {
			addToken(Token(Token::opMultiply, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '+') {
			addToken(Token(Token::opPlus, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '^') {
			addToken(Token(Token::opPower, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '-') {
			addToken(Token(Token::opMinus, i, i + 1, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '<') {
			QString::iterator start = i;
			i++;
			if (*i == '=') {
				addToken(Token(Token::opLessEqual, start, ++i, line, curFilePath));
				continue;
			}
			if (*i == '>') {
				addToken(Token(Token::opNotEqual, start, ++i, line, curFilePath));
				continue;
			}
			addToken(Token(Token::opLess, start, i, line, curFilePath));
			continue;
		}
		if (*i == '>') {
			QString::iterator start = i;
			i++;
			if (*i == '=') {
				addToken(Token(Token::opGreaterEqual, start, ++i, line, curFilePath));
				continue;
			}
			addToken(Token(Token::opGreater, start, i, line, curFilePath));
			continue;
		}
		if (*i == '=') {
			QString::iterator start = i;
			i++;
			if (i != code.end()) {
				if (*i == '>') {
					addToken(Token(Token::opGreaterEqual, start, ++i, line, curFilePath));
					continue;
				}
				if (*i == '<') {
					addToken(Token(Token::opLessEqual, start, ++i, line, curFilePath));
					continue;
				}
			}
			addToken(Token(Token::opEqual, start, i, line, curFilePath));
			continue;
		}
		/*if (*i == '\\') {
			addToken(Token(Token::opTypePtrField, i, i + 1, line, curFilePath));
			i++;
			continue;
		}*/
		if (*i == '.') {
			i++;
			if (i != code.end()) {
				if (i->isNumber()) { //Float
					i--;
					readFloatDot(i, code.end(), line, curFilePath);
					continue;
				}
			}
			addToken(Token(Token::opTypePtrField, i - 1, i, line, curFilePath));
			continue;
		}
		if (*i == '$') {
			i++;
			readHex(i, code.end(), line, curFilePath);
			continue;
		}
		if (i->isDigit()) {
			readNum(i, code.end(), line, curFilePath);
			continue;
		}
		if (i->isLetter() || *i == '_') {
			ReturnState retState = readIdentifier(i, code.end(), line, curFilePath);
			if (retState == Error) return Error;
			if (retState == ErrorButContinue) state = ErrorButContinue;
			continue;
		}

		emit error(ErrorCodes::ecUnexpectedCharacter, tr("Unexpected character \"%1\" %2,%3").arg(QString(*i), QString::number(i->row()), QString::number(i->cell())), line, curFilePath);
		state = ErrorButContinue;
		++i;
	}

	QDir::setCurrent(oldPath);
	return state;
}

void Lexer::addToken(const Token &tok) {
	//tok.print();
	mTokens.append(tok);
}


Lexer::ReturnState Lexer::readToEOL(QString::iterator &i, const QString::iterator &end) {
	while (i != end) {
		if (*i == '\n') {
			return Success;
		}
		i++;
	}
	return Success;
}

Lexer::ReturnState Lexer::readToRemEnd(QString::iterator &i, const QString::iterator &end, int &line, const QString &file) {
	const char * const endRem = "remend";
	int foundIndex = 0;
	while (i != end) {
		if (i->toLower() == endRem[foundIndex]) {
			foundIndex++;
			if (foundIndex == 6) {
				i++;
				return Success;
			}
		}
		else {
			foundIndex = 0;
		}
		if (*i == '\n') {
			line++;
		}
		i++;
	}
	emit warning(ErrorCodes::ecExpectingRemEndBeforeEOF, tr("Expecting RemEnd before end of file"), line, file);
	return Lexer::Error;
}

Lexer::ReturnState Lexer::readFloatDot(QString::iterator &i, const QString::iterator &end, int line, const QString &file) {
	QString::iterator begin = i;
	i++;
	while (i != end) {
		if (!(i->isDigit())) { //Not a number
			break;
		}
		i++;
	}
	if (i->toLower() == 'e') {
		i++;
		if (*i == '-' || *i == '+') {
			i++;
		}
		while (i != end) {
			if (!(i->isDigit())) { //Not a number
				break;
			}
			i++;
		}
	}
	addToken(Token(Token::Float, begin, i, line, file));
	return Success;
}

Lexer::ReturnState Lexer::readNum(QString::iterator &i, const QString::iterator &end, int line, const QString &file) {
	QString::iterator begin = i;
	while (i != end) {
		if (!(i->isDigit())) { //Not a number
			break;
		}
		i++;
	}
	if (*i == '.') { //Float
		i++;
		while (i != end) {
			if (!(i->isDigit())) { //Not a number
				break;
			}
			i++;
		}
		if (i->toLower() == 'e') {
			i++;
			if (*i == '-' || *i == '+') {
				i++;
			}
			while (i != end) {
				if (!(i->isDigit())) { //Not a number
					break;
				}
				i++;
			}
		}
		addToken(Token(Token::Float, begin, i, line, file));
		return Success;
	}

	addToken(Token(Token::Integer, begin, i, line, file));
	return Success;
}


Lexer::ReturnState Lexer::readHex(QString::iterator &i, const QString::iterator &end, int line, const QString &file) {
	QString::iterator begin = i;
	while (i != end) {
		if (!(i->isDigit() || (i->toLower() >= QChar('a') && i->toLower() <= QChar('f')))) { //Not hex
			addToken(Token(Token::IntegerHex, begin, i, line, file));
			return Success;
		}
		i++;
	}
	return Success;
}

Lexer::ReturnState Lexer::readString(QString::iterator &i, const QString::iterator &end, int &line, const QString &file) {
	++i;
	QString::iterator begin = i;
	while (i != end) {
		if (*i == '"') {
			addToken(Token(Token::String, begin, i, line, file));
			i++;
			return Success;
		}
		if (*i == '\n') {
			line++;
		}
		i++;
	}
	error(ErrorCodes::ecExpectingEndOfString, tr("Expecting '\"' before end of file"), line, file);
	return ErrorButContinue;
}

Lexer::ReturnState Lexer::readIdentifier(QString::iterator &i, const QString::iterator &end, int &line, const QString &file) {
	QString::iterator begin = i;
	QString name;
	*i = i->toLower();
	name += *i;
	i++;
	while (i != end) {
		if (!i->isLetterOrNumber() && *i != '_') {
			break;
		}
		*i = i->toLower(); //Lower
		name += *i;
		i++;
	}
	if (name == "remstart") {
		return readToRemEnd(i, end, line, file);
	}
	QMap<QString, Token::Type>::ConstIterator keyIt = mKeywords.find(name);
	if (keyIt != mKeywords.end()) {
		if (keyIt.value() == Token::kInclude) {
			QString includeFile;
			while (i != end) {
				if (*i == '"') {
					i++;
					while (i != end) {
						if (*i == '"') {
							i++;
							ReturnState state =  tokenize(includeFile);
							return (state == Success) ? Success : ErrorButContinue;
						}
						if (*i == '\n') { //Only for correct line number in error message
							line++;
						}
						includeFile += *i;
						i++;
					}
					emit error(ErrorCodes::ecExpectingEndOfString, tr("Expecting '\"' before end of file"), line, file);
					return ErrorButContinue;
				}
				if (i->category() != QChar::Separator_Space) {
					emit error(ErrorCodes::ecExpectingString, tr("Expecting \" after Include"), line, file);
					return Error;
				}
				i++;
			}
		}
		else {
			addToken(Token(keyIt.value(),begin, i, line, file));
			return Success;
		}
	}
	else {
		addToken(Token(Token::Identifier, begin, i, line, file));
	}

	if (i != end) {
		if (*i == '%') {
			addToken(Token(Token::IntegerTypeMark, i, i + 1, line, file));
			i++;
			return Success;
		}
		if (*i == '#') {
			addToken(Token(Token::FloatTypeMark, i, i + 1, line, file));
			i++;
			return Success;
		}
		if (*i == '$') {
			addToken(Token(Token::StringTypeMark, i, i + 1, line, file));
			i++;
			return Success;
		}
	}
	return Success;
}


void Lexer::combineTokens() {
	QList<Token>::Iterator i = mTokens.begin();
	QList<Token>::Iterator last;
	while (i != mTokens.end()) {
		if (i->mType == Token::kEnd) {
			last = i;
			i++;
			if (i != mTokens.end()) {
				if (i->mType == Token::kFunction) {
					int line = i->mLine;
					const QString &file = i->mFile;
					QString::ConstIterator begin = last->mBegin;
					QString::ConstIterator end = i->mEnd;
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndFunction, begin, end, line, file));
					i++;
					continue;
				}
				if (i->mType == Token::kIf) {
					int line = i->mLine;
					const QString &file = i->mFile;
					QString::ConstIterator begin = last->mBegin;
					QString::ConstIterator end = i->mEnd;
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndIf, begin, end, line, file));
					i++;
					continue;
				}
				if (i->mType == Token::kSelect) {
					int line = i->mLine;
					const QString &file = i->mFile;
					QString::ConstIterator begin = last->mBegin;
					QString::ConstIterator end = i->mEnd;
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndSelect, begin, end, line, file));
					i++;
					continue;
				}
				if (i->mType == Token::kType) {
					int line = i->mLine;
					const QString &file = i->mFile;
					QString::ConstIterator begin = last->mBegin;
					QString::ConstIterator end = i->mEnd;
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndType, begin, end, line, file));
					i++;
					continue;
				}
				continue;
			}
			else {
				return;
			}
		}
		if (i->mType == Token::EOL) {
			i++;
			if (i == mTokens.end()) return;
			if (i->mType == Token::Identifier) {
				last = i;
				i++;
				if (i == mTokens.end()) return;
				if (i->mType == Token::Colon) {
					int line = i->mLine;
					const QString &file = i->mFile;
					QString::ConstIterator begin = last->mBegin;
					QString::ConstIterator end = last->mEnd;
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::Label, begin, end, line, file));
					i++;
					continue;
				}
			}
			continue;
		}
		i++;

	}
}


void Lexer::printTokens() {
	for (QList<Token>::const_iterator i = mTokens.begin(); i != mTokens.end(); i++) {
		i->print();
	}
}

void Lexer::writeTokensToFile(const QString &fileName) {
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		emit error(ErrorCodes::ecCantOpenFile, tr("Cannot open file %1").arg(fileName), 0, 0);
		return;
	}
	QTextStream out(&file);
	out << mTokens.size() << " tokens\n\n";
	for (QList<Token>::const_iterator i = mTokens.begin(); i != mTokens.end(); i++) {
		out << i->info() << '\n';
	}
}
