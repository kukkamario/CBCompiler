#ifndef CUSTOMDATATYPEDEFINITIONS_H
#define CUSTOMDATATYPEDEFINITIONS_H
#include <string>
#include <vector>
#include "codepoint.h"
class ErrorHandler;
class CustomDataTypeDefinitions {
		Q_OBJECT
	public:
		struct CustomDataType {
				/** CB type name*/
				std::string mName;

				/** LLVM type name*/
				std::string mDataType;
		};

		CustomDataTypeDefinitions(ErrorHandler *errorHandler);
		~CustomDataTypeDefinitions();

		bool parse(const std::string &file);
		const std::vector<CustomDataType> &dataTypes() const { return mDataTypes; }

	private:
		void invalidFormatError(const std::string &file);
		bool parseDataType(QJsonObject obj, CustomDataType &ret);
		void error(int code, std::string msg, CodePoint cp);
		void warning(int code, std::string msg, CodePoint cp);


		std::vector<CustomDataType> mDataTypes;
		ErrorHandler *mErrorHandler;
};

#endif // CUSTOMDATATYPEDEFINITIONS_H
