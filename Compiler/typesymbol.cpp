#include "typesymbol.h"
#include "typepointervaluetype.h"
#include "llvm.h"
#include "runtime.h"


TypeSymbol::TypeSymbol(const QString &name, QFile *file, int line):
	Symbol(name, file, line),
	mType(0),
	mTypePointerValueType(0){
}

bool TypeSymbol::addField(const TypeField &field) {
	if (mFieldSearch.contains(field.name())) {
		return false;
	}
	mFields.append(field);
	mFieldSearch.insert(field.name(), --mFields.end());
	return true;
}

bool TypeSymbol::hasField(const QString &name) {
	return mFieldSearch.contains(name);
}

const TypeField &TypeSymbol::field(const QString &name) const{
	QMap<QString, QLinkedList<TypeField>::Iterator>::ConstIterator i = mFieldSearch.find(name);
	assert(i != mFieldSearch.end());
	return *i.value();
}

bool TypeSymbol::createLLVMType(llvm::Module *mod) {

	std::vector<llvm::Type*> members;
	members.push_back(llvm::Type::getInt32Ty(mod->getContext())->getPointerTo()); //Pointer to type
	members.push_back(llvm::Type::getInt32Ty(mod->getContext())->getPointerTo()); //last
	members.push_back(llvm::Type::getInt32Ty(mod->getContext())->getPointerTo()); //next
	for (QLinkedList<TypeField>::ConstIterator i = mFields.begin(); i != mFields.end(); i++) {
		const TypeField &field = *i;
		members.push_back(field.valueType()->llvmType());
	}
	mMemberType = llvm::StructType::create(mod->getContext(), members,("CBType_" + mName).toStdString());
	if (!mMemberType) return false;

	//TODO TYPE

	return true;
}


TypeField::TypeField(const QString &name, ValueType *valueType, QFile *file, int line) :
	mName(name),
	mValueType(valueType),
	mLine(line),
	mFile(file){
}

QString TypeField::info() const {
	return QString("Field %1 %2").arg(mValueType->name(), mName);
}


bool TypeSymbol::createTypePointerValueType(Runtime *r) {
	if (!createLLVMType(r->module())) return false;
	mTypePointerValueType = new TypePointerValueType(r, this);
	return true;
}


QString TypeSymbol::info() const {
	QString str("Type %1\n");
	str = str.arg(mName);
	for (QLinkedList<TypeField>::ConstIterator i = mFields.begin(); i != mFields.end(); i++) {
		str += "    " + i->info() + '\n';
	}
	return str;
}
