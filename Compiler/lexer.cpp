#include "lexer.h"
#include "errorcodes.h"
#include <iostream>
#include <fstream>
#include <clocale>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;

utf8_iterator operator+(utf8_iterator it, int i) {
	if (i > 0) {
		do {
			++it;
		} while(--i);
	} else if (i < 0) {
		do {
			--it;
		} while(++i);
	}
	return it;
}

utf8_iterator operator-(utf8_iterator it, int i) {
	if (i > 0) {
		do {
			--it;
		} while(--i);
	} else if (i < 0) {
		do {
			++it;
		} while(++i);
	}
	return it;
}

Lexer::Lexer(ErrorHandler *errorHandler) :
	mErrorHandler(errorHandler)
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
	mKeywords["struct"] = Token::kStruct;
	mKeywords["field"] = Token::kField;
	mKeywords["endtype"] = Token::kEndType;
	mKeywords["endstruct"] = Token::kEndStruct;
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
	mKeywords["delete"] = Token::kDelete;
	mKeywords["arraysize"] = Token::kArraySize;

	mKeywords["include"] = Token::kInclude;
}

Lexer::ReturnState Lexer::tokenizeFile(const std::string &file, Settings *settings) {
	mSettings = settings;

	Lexer::ReturnState ret = tokenize(file);
	if (ret == Lexer::Error) return Lexer::Error;

	combineTokens();

	//Dirty trick, but works
	mTokens.push_back(Token(Token::EOL, std::string(), CodePoint()));
	mTokens.push_back(Token(Token::EOL, std::string(), CodePoint()));
	mTokens.push_back(Token(Token::EndOfTokens, std::string(), CodePoint()));
	return ret;
}

Lexer::ReturnState Lexer::tokenize(const std::string &file) {
	std::ifstream curFile(file);
	if (!curFile.is_open()) {
		mFiles.push_back(std::pair<std::string, std::string>(file, ""));
		error(ErrorCodes::ecCantOpenFile, "Cannot open file " + file, CodePoint());
		return Error;
	}
	std::cout << "File \"" << file << "\" opened\n";
	fs::path oldPath = fs::current_path();
	fs::path folderPath = fs::absolute(fs::path(file)).remove_filename();
	fs::current_path(folderPath);

	std::string curFilePath = file;

	std::string code2;
	std::string lineStr;
	while (std::getline(curFile, lineStr)) {
		code2 += lineStr;
		code2 += '\n';
	}

	curFile.close();
	mFiles.push_back(std::pair<std::string, std::string>(file, code2));
	std::string &code = mFiles.back().second;

	ReturnState state = Success;
	int line = 1;
	utf8_iterator codeStart(code.begin(), code.begin(), code.end());
	utf8_iterator codeEnd(code.end(), code.begin(), code.end());
	utf8_iterator lineStart = codeStart;
	for (utf8_iterator i = codeStart; i != codeEnd;) {
		if (isSpace(i)) { // Space
			i++;
			continue;
		}
		if (*i == '\'') { //Single line comment
			i++;
			if (i == codeEnd) return state;
			readToEOL(i, codeEnd);
			if (i != codeEnd) {
				addToken(Token::EOL, i, i + 1, codePoint(i, lineStart, line, curFilePath));
				++i;
				line++;
				lineStart = i;
			}
			continue;
		}
		if (*i == '/') {
			i++;
			if (i != codeEnd) {
				if (*i == '/') {
					i++;
					readToEOL(i, codeEnd);
					addToken(Token::EOL, i, i + 1, codePoint(i, lineStart, line, curFilePath));
					i++;
					line++;
					lineStart = i;
					continue;
				}
			}
			i--;
			addToken(Token::opDivide, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '\n') {
			addToken(Token::EOL, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			line++;
			lineStart = i;
			continue;
		}
		if (*i == ',') {
			addToken(Token::Comma, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == ':') {
			addToken(Token::Colon, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '(') {
			addToken(Token::LeftParenthese, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == ')') {
			addToken(Token::RightParenthese, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '[') {
			addToken(Token::LeftSquareBracket, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == ']') {
			addToken(Token::RightSquareBracket, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '"') {
			readString(i, codeEnd, lineStart, line, curFilePath);
			continue;
		}
		if (*i == '*') {
			addToken(Token::opMultiply, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '+') {
			addToken(Token::opPlus, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '^') {
			addToken(Token::opPower, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '-') {
			addToken(Token::opMinus, i, i + 1, codePoint(i, lineStart, line, curFilePath));
			i++;
			continue;
		}
		if (*i == '<') {
			utf8_iterator start = i;
			i++;
			if (*i == '=') {
				++i;
				addToken(Token::opLessEqual, start, i, codePoint(i, lineStart, line, curFilePath));
				continue;
			}
			if (*i == '>') {
				++i;
				addToken(Token::opNotEqual, start, i, codePoint(i, lineStart, line, curFilePath));
				continue;
			}
			addToken(Token::opLess, start, i, codePoint(i, lineStart, line, curFilePath));
			continue;
		}
		if (*i == '>') {
			utf8_iterator begin = i;
			i++;
			if (*i == '=') {
				++i;
				addToken(Token::opGreaterEqual, begin, i, codePoint(begin, lineStart, line, curFilePath));
				continue;
			}
			addToken(Token::opGreater, begin, i, codePoint(begin, lineStart, line, curFilePath));
			continue;
		}
		if (*i == '=') {
			utf8_iterator begin = i;
			i++;
			if (i != codeEnd) {
				if (*i == '>') {
					++i;
					addToken(Token::opGreaterEqual, begin, i, codePoint(begin, lineStart, line, curFilePath));
					continue;
				}
				if (*i == '<') {
					++i;
					addToken(Token::opLessEqual, begin, i, codePoint(begin, lineStart, line, curFilePath));
					continue;
				}
			}
			addToken(Token::opEqual, begin, i, codePoint(begin, lineStart, line, curFilePath));
			continue;
		}
		if (*i == '.') {
			i++;
			if (i != codeEnd) {
				if (isDigit(i)) { //Float
					i--;
					readFloatDot(i, codeEnd, lineStart, line, curFilePath);
					continue;
				}
			}
			addToken(Token::opDot, i - 1, i, codePoint(i - 1, lineStart, line, curFilePath));
			continue;
		}
		if (*i == '$') {
			i++;
			readHex(i, codeEnd, lineStart, line, curFilePath);
			continue;
		}
		if (isDigit(i)) {
			readNum(i, codeEnd, lineStart, line, curFilePath);
			continue;
		}
		if (isAlpha(i) || *i == '_') {
			ReturnState retState = readIdentifier(i, codeEnd, lineStart, line, curFilePath);
			if (retState == Error) return Error;
			if (retState == ErrorButContinue) state = ErrorButContinue;
			continue;
		}
		char characterString[5] = {0, 0, 0, 0, 0};
		utf8::append(*i, characterString);
		error(ErrorCodes::ecUnexpectedCharacter, (boost::format("Unexpected character '%s' %x") % characterString % *i).str(), codePoint(i, lineStart, line, file));
		state = ErrorButContinue;
		++i;
	}

	fs::current_path(oldPath);
	return state;
}

void Lexer::addToken(const Token &tok) {
	//tok.print();
	mTokens.push_back(tok);
}

void Lexer::addToken(Token::Type type, utf8_iterator begin, utf8_iterator end, const CodePoint &cp) {
	addToken(Token(type, std::string(begin.base(), end.base()), cp));
}


Lexer::ReturnState Lexer::readToEOL(utf8_iterator &i, const utf8_iterator &end) {
	while (i != end) {
		if (*i == '\n') {
			return Success;
		}
		i++;
	}
	return Success;
}

Lexer::ReturnState Lexer::readToEndRem(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int &line, const std::string &file) {
	const char * const endRem = "endrem";
	int foundIndex = 0;
	while (i != end) {
		if (*i < 127 && std::tolower(*i) == endRem[foundIndex]) {
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
			lineStart = i + 1;
		}
		i++;
	}
	warning(ErrorCodes::ecExpectingRemEndBeforeEOF, "Expecting EndRem before end of file", codePoint(i - 1, lineStart, line, file));
	return Lexer::Error;
}

Lexer::ReturnState Lexer::readFloatDot(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int line, const std::string &file) {
	utf8_iterator begin = i;
	i++;
	while (i != end) {
		if (!isDigit(i)) { //Not a number
			break;
		}
		i++;
	}
	if (*i == 'e' || *i == 'E') {
		i++;
		if (*i == '-' || *i == '+') {
			i++;
		}
		while (i != end) {
			if (!isDigit(i)) { //Not a number
				break;
			}
			i++;
		}
	}
	addToken(Token::Float, begin, i, codePoint(begin, lineStart, line, file));
	return Success;
}

Lexer::ReturnState Lexer::readNum(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int line, const std::string &file) {
	utf8_iterator begin = i;
	while (i != end) {
		if (!isDigit(i)) { //Not a number
			break;
		}
		i++;
	}
	if (*i == '.') { //Float
		i++;
		while (i != end) {
			if (!isDigit(i)) { //Not a number
				break;
			}
			i++;
		}
		if (*i == 'e' || *i == 'E') {
			i++;
			if (*i == '-' || *i == '+') {
				i++;
			}
			while (i != end) {
				if (!isDigit(i)) { //Not a number
					break;
				}
				i++;
			}
		}
		addToken(Token::Float, begin, i, codePoint(begin, lineStart, line, file));
		return Success;
	}

	addToken(Token::Integer, begin, i, codePoint(begin, lineStart, line, file));
	return Success;
}


Lexer::ReturnState Lexer::readHex(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int line, const std::string &file) {
	utf8_iterator begin = i;
	while (i != end) {
		if (!(isDigit(i) || (*i >= 'a' && *i <= 'f') || (*i >= 'A' && *i <= 'F'))) { //Not hex
			addToken(Token::IntegerHex, begin, i, codePoint(begin, lineStart, line, file));
			return Success;
		}
		i++;
	}
	return Success;
}

Lexer::ReturnState Lexer::readString(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int &line, const std::string &file) {
	utf8_iterator begin = i;
	++i;
	while (i != end) {
		if (*i == '"') {
			addToken(Token::String, begin + 1, i, codePoint(begin, lineStart, line, file));
			i++;
			return Success;
		}
		if (*i == '\n') {
			line++;
			lineStart = i + 1;
		}
		i++;
	}
	error(ErrorCodes::ecExpectingEndOfString, "Expecting '\"' before end of file", codePoint(i - 1, lineStart, line, file));
	return ErrorButContinue;
}

Lexer::ReturnState Lexer::readIdentifier(utf8_iterator &i, const utf8_iterator &end, utf8_iterator &lineStart, int &line, const std::string &file) {
	utf8_iterator begin = i;
	std::string name;
	name += *i;
	i++;
	while (i != end) {
		char32_t c = *i;
		if (!std::isalnum(c, std::locale()) && c != '_') {
			break;
		}
		name += *i;
		i++;
	}
	if (name == "rem") {
		return readToEndRem(i, end, lineStart, line, file);
	}
	std::map<std::string, Token::Type>::const_iterator keyIt = mKeywords.find(name);
	if (keyIt != mKeywords.end()) {
		if (keyIt->second == Token::kInclude) {
			std::string includeFile;
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
							lineStart = i + 1;
						}
						includeFile += *i;
						i++;
					}
					error(ErrorCodes::ecExpectingEndOfString, "Expecting '\"' before end of file", codePoint(i - 1, lineStart, line, file));
					return ErrorButContinue;
				}
				if (isSpace(i)) {
					error(ErrorCodes::ecExpectingString, "Expecting \" after Include", codePoint(i, lineStart, line, file));
					return Error;
				}
				i++;
			}
		}
		else {
			addToken(keyIt->second,begin, i, codePoint(begin, lineStart, line, file));
			return Success;
		}
	}
	else {
		addToken(Token::Identifier, begin, i, codePoint(begin, lineStart, line, file));
	}

	if (i != end) {
		if (*i == '%') {
			addToken(Token::IntegerTypeMark, i, i + 1, codePoint(i, lineStart, line, file));
			i++;
			return Success;
		}
		if (*i == '#') {
			addToken(Token::FloatTypeMark, i, i + 1, codePoint(i, lineStart, line, file));
			i++;
			return Success;
		}
		if (*i == '$') {
			addToken(Token::StringTypeMark, i, i + 1, codePoint(i, lineStart, line, file));
			i++;
			return Success;
		}
	}
	return Success;
}

CodePoint Lexer::codePoint(utf8_iterator i, utf8_iterator lineStart, int line, const std::string &file) {
	return CodePoint(line, utf8::distance(lineStart.base(), i.base()) + 1, file);
}

bool Lexer::isDigit(utf8_iterator i) {
	char32_t c = *i;
	return std::isdigit(c, std::locale());
}

bool Lexer::isAlnum(utf8_iterator i) {
	char32_t c = *i;
	return std::isalnum(c, std::locale());
}

bool Lexer::isAlpha(utf8_iterator i) {
	char32_t c = *i;
	return std::isalpha(c, std::locale());
}

bool Lexer::isSpace(utf8_iterator i) {
	return *i == ' ' || *i == char(9); /* horizontal tab */
}

void Lexer::error(int code, std::string msg, CodePoint cp) {
	mErrorHandler->error(code, msg, cp);
}

void Lexer::warning(int code, std::string msg, CodePoint cp) {
	mErrorHandler->warning(code, msg, cp);
}


void Lexer::combineTokens() {
	std::vector<Token>::iterator i = mTokens.begin();
	std::vector<Token>::iterator last;
	while (i != mTokens.end()) {
		if (i->type() == Token::kEnd) {
			last = i;
			i++;
			if (i != mTokens.end()) {
				if (i->type() == Token::kFunction) {
					std::string text =  last->toString() + ' ' + i->toString();
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndFunction, text, last->codePoint()));
					i++;
					continue;
				}
				if (i->type() == Token::kIf) {
					std::string text =  last->toString() + ' ' + i->toString();
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndIf, text, last->codePoint()));
					i++;
					continue;
				}
				if (i->type() == Token::kSelect) {
					std::string text =  last->toString() + ' ' + i->toString();
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndSelect, text, last->codePoint()));
					i++;
					continue;
				}
				if (i->type() == Token::kType) {
					std::string text =  last->toString() + ' ' + i->toString();
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndType, text, last->codePoint()));
					i++;
					continue;
				}
				if (i->type() == Token::kStruct) {
					std::string text =  last->toString() + ' ' + i->toString();
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::kEndStruct, text, last->codePoint()));
					i++;
					continue;
				}
				continue;
			}
			else {
				return;
			}
		}
		if (i->type() == Token::EOL) {
			i++;
			if (i == mTokens.end()) return;
			if (i->type() == Token::Identifier) {
				last = i;
				i++;
				if (i == mTokens.end()) return;
				if (i->type() == Token::Colon) {
					i++;
					i = mTokens.erase(last, i);
					i = mTokens.insert(i, Token(Token::Label, last->toString(), last->codePoint()));
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
	for (std::vector<Token>::const_iterator i = mTokens.begin(); i != mTokens.end(); i++) {
		i->print();
	}
}

void Lexer::writeTokensToFile(const std::string &fileName) {
	std::ofstream out(fileName);
	if (!out.is_open()) {
		error(ErrorCodes::ecCantOpenFile, "Cannot open file " + fileName, CodePoint());
		return;
	}
	out << mTokens.size() << " tokens\n\n";
	for (std::vector<Token>::const_iterator i = mTokens.begin(); i != mTokens.end(); i++) {
		out << i->info() << '\n';
	}
}
