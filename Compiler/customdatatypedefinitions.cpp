#include "customdatatypedefinitions.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QFile>
#include "errorcodes.h"

CustomDataTypeDefinitions::CustomDataTypeDefinitions(ErrorHandler *errorHandler) :
	mErrorHandler(errorHandler) {
}

CustomDataTypeDefinitions::~CustomDataTypeDefinitions()
{
}

bool CustomDataTypeDefinitions::parse(const std::string &file) {

	QByteArray jsonText;
	QFile jsonFile(file);
	if (!jsonFile.open(QIODevice::ReadOnly)) {
		error(ErrorCodes::ecCantOpenFile, tr("Can't open a custom data type definition file \"%1\"").arg(file), CodePoint(0, 0, file));
		return false;
	}
	jsonText = jsonFile.readAll();


	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(jsonText, &parseError);

	if (doc.isNull()) {
		error(ErrorCodes::ecCantParseCustomDataTypeDefinitionFile, tr("Can't parse a custom data type definition file:    %1").arg(parseError.errorString()), CodePoint(0, 0, file));
		return false;
	}


	if (!doc.isArray()) {
		invalidFormatError(file);
		return false;
	}

	QJsonArray array = doc.array();
	foreach(QJsonValue val, array) {
		if (!val.isObject()) {
			invalidFormatError(file);
		}
		QJsonObject obj = val.toObject();
		CustomDataType dataType;
		if (!parseDataType(obj, dataType)) {
			invalidFormatError(file);
			return false;
		}
		mDataTypes.push_back(dataType);
	}
	return true;
}


void CustomDataTypeDefinitions::invalidFormatError(const std::string &file) {
	error(ErrorCodes::ecInvalidCustomDataTypeDefinitionFileFormat, tr("Invalid custom data type file format"), CodePoint(0, 0, file));
}

bool CustomDataTypeDefinitions::parseDataType(QJsonObject obj, CustomDataTypeDefinitions::CustomDataType &ret) {
	if (!(obj.contains("name") && obj.contains("type"))) {
		return false;
	}

	QJsonValue name = obj.take("name");
	QJsonValue dataType = obj.take("type");

	if (!(name.isString() && dataType.isString())) {
		return false;
	}
	ret.mName = name.toString();
	ret.mDataType = dataType.toString();

	return true;
}

void CustomDataTypeDefinitions::error(int code, std::string msg, CodePoint cp) {
	mErrorHandler->error(code, msg, cp);
}

void CustomDataTypeDefinitions::warning(int code, std::string msg, CodePoint cp) {
	mErrorHandler->warning(code, msg, cp);
}
