#include "abstractsyntaxtree.h"
#include <QDebug>
#include <QByteArray>
#include <QFile>
#include <QTextStream>

namespace ast {

const char *const operatorStrings[] = {
	"opEqual",
	"opNotEqual",
	"opGreater",
	"opLess",
	"opGreaterEqual",
	"opLessEqual",
	"opPlus",
	"opMinus",
	"opMultiply",
	"opPower",
	"opMod",
	"opShl",
	"opShr",
	"opSar",
	"opDivide",
	"opAnd",
	"opOr",
	"opXor",
	"opNot",
	"opInvalid"
};

QString operatorToString(Operator op) {
	if (op >= opEqual && op < opInvalid) {
		return operatorStrings[op];
	}
	return operatorStrings[opInvalid];
}

void Printer::printNode(const Node *s, int tab) {
	switch(s->type()) {
		case Node::ntBlock:
			printBlock((Block*)s, tab); return;
		case Node::ntIfStatement:
			printIfStatement((IfStatement*)s, tab); return;
		case Node::ntTypeDefinition:
			printTypeDefinition((TypeDefinition*)s, tab); return;
		case Node::ntWhileStatement:
			printWhileStatement((WhileStatement*)s, tab); return;
		case Node::ntForToStatement:
			printForToStatement((ForToStatement*)s, tab); return;
		case Node::ntForEachStatement:
			printForEachStatement((ForEachStatement*)s, tab); return;
		case Node::ntRepeatForeverStatement:
			printRepeatForeverStatement((RepeatForeverStatement*)s, tab); return;
		case Node::ntRepeatUntilStatement:
			printRepeatUntilStatement((RepeatUntilStatement*)s, tab); return;
		case Node::ntExpression:
			printExpression((Expression*)s, tab); return;
		case Node::ntCommandCall:
			printCommandCall((CommandCall*)s, tab); return;
		case Node::ntArrayDefinition:
			printArrayDefinition((ArrayDefinition*)s, tab); return;
		case Node::ntVariableDefinition:
			printVariableDefinition((VariableDefinition*)s, tab); return;
		case Node::ntGlobalDefinition:
			printGlobalDefinition((GlobalDefinition*)s, tab); return;
		case Node::ntAssignmentExpression:
			printAssignmentExpression((AssignmentExpression*)s, tab); return;
		case Node::ntUnary:
			printUnary((Unary*)s, tab); return;
		case Node::ntReturn:
			printReturn((Return*)s, tab); return;
		case Node::ntTypePtrField:
			printTypePtrField((TypePtrField*)s, tab); return;
		case Node::ntVariable:
			printVariable((Variable*)s, tab); return;
		case Node::ntInteger:
			printInteger((Integer*)s, tab); return;
		case Node::ntFloat:
			printFloat((Float*)s, tab); return;
		case Node::ntString:
			printString((String*)s, tab); return;
		case Node::ntFunctionCallOrArraySubscript:
			printFunctionCallOrArraySubscript((FunctionCallOrArraySubscript*)s, tab); return;
		case Node::ntNew:
			printNew((New*)s, tab); return;
	}
}

void Printer::printBlock(const Block *s, int tab){
	for (Block::ConstIterator i = s->begin(); i != s->end(); i++) {
		printNode(*i);
	}
}

void Printer::printTypeDefinition(const TypeDefinition *s, int tab) {

}

void Printer::printIfStatement(const IfStatement *s, int tab)
{
}

void Printer::printWhileStatement(const WhileStatement *s, int tab)
{
}

void Printer::printForToStatement(const ForToStatement *s, int tab)
{
}

void Printer::printForEachStatement(const ForEachStatement *s, int tab)
{
}

void Printer::printRepeatForeverStatement(const RepeatForeverStatement *s, int tab)
{
}

void Printer::printRepeatUntilStatement(const RepeatUntilStatement *s, int tab) {

}

void Printer::printExpression(const Expression *s, int tab) {
	printNode(s->mFirst, tab + 1);
	for (QList<Operation>::ConstIterator i = s->mRest.begin(); i != s->mRest.end(); i++) {
		printLine(operatorToString(i->mOperator), tab);
		printNode(i->mOperand, tab + 1);
	}
}

void Printer::printCommandCall(const CommandCall *s, int tab) {
	printLine(s->mName, tab);
	tab++;
	for (QList<Node*>::ConstIterator i = s->mParams.begin(); i != s->mParams.end(); i++) {
		printNode(*i, tab);
	}
}

void Printer::printArrayDefinition(const ArrayDefinition *s, int tab) {
}

void Printer::printVariableDefinition(const VariableDefinition *s, int tab) {
	printLine("Dim", tab);
	tab++;
	for (QList<Variable*>::ConstIterator i = s->mDefinitions.begin(); i != s->mDefinitions.end(); i++) {
		printVariable(*i, tab);
	}
}

void Printer::printGlobalDefinition(const GlobalDefinition *s, int tab) {
	printLine("Global", tab);
	tab++;
	for (QList<Variable*>::ConstIterator i = s->mDefinitions.begin(); i != s->mDefinitions.end(); i++) {
		printVariable(*i, tab);
	}
}

void Printer::printAssignmentExpression(const AssignmentExpression *s, int tab) {
	printLine("Assign", tab);
	tab++;
	printVariable(s->mVariable, tab);
	printNode(s->mExpression, tab);
}

void Printer::printUnary(const Unary *s, int tab) {
	printLine(operatorToString(s->mOperator), tab);
	tab++;
	printNode(s->mOperand);
}

void Printer::printReturn(const Return *s, int tab)
{
}

void Printer::printTypePtrField(const TypePtrField *s, int tab) {
}

void Printer::printVariable(const Variable *s, int tab) {
	QString txt;
	txt.reserve(s->mName.size() + s->mTypeName.size() + 10);
	txt += s->mName;
	switch (s->mVarType) {
		case Variable::Integer:
			txt += " As Integer"; break;
		case Variable::Float:
			txt += " As Float"; break;
		case Variable::String:
			txt += " As String"; break;
		case Variable::Short:
			txt += " As Short"; break;
		case Variable::Byte:
			txt += " As Byte"; break;
		case Variable::TypePtr:
			txt += '.'; txt += s->mTypeName; break;
	}
	printLine(txt, tab);
}

void Printer::printInteger(const Integer *s, int tab) {
	printLine(QString::number(s->mValue), tab);
}

void Printer::printFloat(const Float *s, int tab) {
	printLine(QString::number(s->mValue), tab);
}

void Printer::printString(const String *s, int tab) {
	QString txt;
	txt.reserve(s->mValue.size() + 3);
	txt += '"';
	txt += s->mValue;
	txt += '"';
	printLine(txt, tab);
}

void Printer::printFunctionCallOrArraySubscript(const FunctionCallOrArraySubscript *s, int tab) {
	printLine(s->mName+ "(", tab);
	tab++;
	for (QList<Node*>::ConstIterator i = s->mParams.begin(); i != s->mParams.end(); i++) {
		printNode(*i, tab);
	}
	tab--;
	printLine(")", tab);
}

void Printer::printNew(const New *s, int tab) {
	QString txt;
	txt += "New (";
	txt += s->mTypeName;
	txt += ")";
	printLine(txt, tab);
}

bool Printer::printToFile(const QString &file) {
	if (mDestFile) {
		delete mDestFile;
	}
	mDestFile = new QFile(file);
	if (!mDestFile->open(QFile::WriteOnly | QFile::Text)) {
		delete mDestFile;
		mDestFile = 0;
		printLine("Cant open file " + file);
		return false;
	}
	return true;
}

void Printer::printLine(const QString &txt, int tab) {
	if (mDestFile) {
		QTextStream out(mDestFile);
		if (tab) {
			QByteArray spaces(tab * 2, ' ');
			out << spaces << txt << '\n';
		}
		else {
			out << txt << '\n';
		}
	}
	else {
		if (tab) {
			QByteArray spaces(tab, ' ');
			qDebug("%s%s", spaces.data(), qPrintable(txt));
			return;
		}
		qDebug("%s", qPrintable(txt));
	}
}

}
