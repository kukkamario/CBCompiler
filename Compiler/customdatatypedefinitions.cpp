#include "customdatatypedefinitions.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QFile>
#include "errorcodes.h"

CustomDataTypeDefinitions::CustomDataTypeDefinitions(QObject *parent) :
	QObject(parent) {
}

CustomDataTypeDefinitions::~CustomDataTypeDefinitions()
{
}

bool CustomDataTypeDefinitions::parse(const QString &file) {

	QByteArray jsonText;
	QFile jsonFile(file);
	if (!jsonFile.open(QIODevice::ReadOnly)) {
		emit error(ErrorCodes::ecCantOpenFile, tr("Can't open a custom data type definition file \"%1\"").arg(file), 0, QString());
		return false;
	}
	jsonText = jsonFile.readAll();


	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(jsonText, &parseError);

	if (doc.isNull()) {
		emit error(ErrorCodes::ecCantParseCustomDataTypeDefinitionFile, tr("Can't parse a custom data type definition file:    %1").arg(parseError.errorString()), 0, file);
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
		mDataTypes.append(dataType);
	}
	return true;
}


void CustomDataTypeDefinitions::invalidFormatError(const QString &file) {
	emit error(ErrorCodes::ecInvalidCustomDataTypeDefinitionFileFormat, tr("Invalid custom data type file format"), 0, file);
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
