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
		case Node::ntConstDefinition:
			printConstDefinition((ConstDefinition*)s, tab); return;
		case Node::ntGlobalDefinition:
			printGlobalDefinition((GlobalDefinition*)s, tab); return;
		case Node::ntAssignmentExpression:
			printAssignmentExpression((AssignmentExpression*)s, tab); return;
		case Node::ntUnary:
			printUnary((Unary*)s, tab); return;
		case Node::ntReturn:
			printReturn((Return*)s, tab); return;
		case Node::ntExit:
			printExit((Exit*)s, tab); return;
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
		case Node::ntArraySubscriptAssignmentExpression:
			printArraySubscriptAssignmentExpression((ArraySubscriptAssignmentExpression*)s, tab); return;
		case Node::ntFunctionDefinition:
			printFunctionDefinition((FunctionDefinition*)s, tab); return;
		default:
			printLine("Unknown AST node " + QString::number(s->type())); return;
	}
}

void Printer::printBlock(const Block *s, int tab){
	for (Block::ConstIterator i = s->begin(); i != s->end(); i++) {
		printNode(*i, tab);
	}
}

void Printer::printTypeDefinition(const TypeDefinition *s, int tab) {
	printLine("Type " + s->mName, tab);
	for (QList<QPair<int, Variable*> >::ConstIterator i = s->mFields.begin(); i != s->mFields.end(); i++) {
		printVariable(i->second, tab + 1);
	}
	printLine("EndType");
}

void Printer::printIfStatement(const IfStatement *s, int tab) {
	printLine("If", tab);
	printNode(s->mCondition, tab + 1);
	printLine("Then", tab);
	printBlock(&s->mIfTrue, tab + 1);
	if (!s->mElse.empty()) {
		printLine("Else", tab);
		printBlock(&s->mElse, tab + 1);
	}
	printLine("EndIf", tab);
}

void Printer::printWhileStatement(const WhileStatement *s, int tab) {
	printLine("While", tab);
	printNode(s->mCondition, tab + 1);
	printBlock(&s->mBlock, tab + 1);
	printLine("Wend", tab);
}

void Printer::printForToStatement(const ForToStatement *s, int tab) {
	printLine("For " + s->mVarName + " as " + varTypeToString(s->mVarType) + " = ", tab);
	printNode(s->mFrom, tab + 1);
	printLine("To", tab);
	printNode(s->mTo, tab + 1);
	if (s->mStep) {
		printLine("Step", tab);
		printNode(s->mStep, tab + 1);
	}
	printBlock(&s->mBlock, tab + 1);
	printLine("Next " + s->mVarName, tab);
}

void Printer::printForEachStatement(const ForEachStatement *s, int tab) {
	printLine("For " + s->mVarName + "\\" + s->mTypeName + " = Each " + s->mTypeName, tab);
	printBlock(&s->mBlock, tab + 1);
	printLine("Next " + s->mVarName);
}

void Printer::printRepeatForeverStatement(const RepeatForeverStatement *s, int tab) {
	printLine("Repeat", tab);
	printBlock(&s->mBlock, tab + 1);
	printLine("Until", tab);
}

void Printer::printRepeatUntilStatement(const RepeatUntilStatement *s, int tab) {
	printLine("Repeat", tab);
	printBlock(&s->mBlock, tab + 1);
	printLine("Until", tab);
	printNode(s->mCondition, tab + 1);

}

void Printer::printExpression(const Expression *s, int tab) {
	printNode(s->mFirst, tab + 1);
	for (QList<Operation>::ConstIterator i = s->mRest.begin(); i != s->mRest.end(); i++) {
		printLine(operatorToString(i->mOperator), tab);
		printNode(i->mOperand, tab + 1);
	}
}

void Printer::printFunctionDefinition(const FunctionDefinition *s, int tab) {
	printLine("Function " + s->mName + "( ", tab);
	if (!s->mParams.isEmpty()) {
		QList<FunctionParametreDefinition>::ConstIterator i = s->mParams.begin();
		printVariable(&i->mVariable, tab + 1);
		if (i->mDefaultValue) {
			printLine("=", tab + 1);
			printNode(i->mDefaultValue, tab + 1);
		}
		i++;
		while (i != s->mParams.end()) {
			printLine(",", tab);
			printVariable(&i->mVariable, tab + 1);
			if (i->mDefaultValue) {
				printLine("=", tab + 1);
				printNode(i->mDefaultValue, tab + 1);
			}
			i++;
		}
	}
	printLine(") as " + varTypeToString(s->mRetType), tab);
	printBlock(&s->mBlock, tab + 1);
	printLine("EndFunction", tab);
}

void Printer::printCommandCall(const CommandCall *s, int tab) {
	printLine(s->mName, tab);
	tab++;
	for (QList<Node*>::ConstIterator i = s->mParams.begin(); i != s->mParams.end(); i++) {
		printNode(*i, tab);
	}
}

void Printer::printArrayDefinition(const ArrayDefinition *s, int tab) {
	printLine("Dim " + s->mName + " as " + varTypeToString(s->mType) + "(", tab);
	tab++;
	QList<Node*>::ConstIterator i = s->mDimensions.begin();
	printNode(*i, tab);
	i++;
	while (i != s->mDimensions.end()) {
		printLine(",", tab - 1);
		printNode(*i, tab);
		i++;
	}
	printLine(")", tab - 1);
}

void Printer::printVariableDefinition(const VariableDefinition *s, int tab) {
	printLine("Dim", tab);
	tab++;
	for (QList<Variable*>::ConstIterator i = s->mDefinitions.begin(); i != s->mDefinitions.end(); i++) {
		printVariable(*i, tab);
	}
}

void Printer::printConstDefinition(const ConstDefinition *s, int tab) {
	printLine("Const " + s->mName + " " + varTypeToString(s->mVarType) + "=", tab);
	tab++;
	printNode(s->mValue, tab);
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
	printNode(s->mVariable, tab);
	printNode(s->mExpression, tab);
}

void Printer::printArraySubscriptAssignmentExpression(const ArraySubscriptAssignmentExpression *s, int tab) {
	printLine("Assign (array)", tab);
	tab++;
	printLine(s->mArrayName + " (", tab);
	tab++;
	QList<Node*>::ConstIterator p = s->mSubscripts.begin();
	printNode(*p, tab);
	p++;
	while (p != s->mSubscripts.end()) {
		printLine(",", tab - 1);
		printNode(*p, tab);
		p++;
	}
	printLine(") =", tab - 1);
	printNode(s->mValue, tab);
}

void Printer::printUnary(const Unary *s, int tab) {
	printLine(operatorToString(s->mOperator), tab);
	tab++;
	printNode(s->mOperand, tab);
}

void Printer::printReturn(const Return *s, int tab) {
	printLine("Return", tab);
	tab++;
	printNode(s->mValue, tab);
}

void Printer::printExit(const Exit *s, int tab) {
	printLine("Exit", tab);
}

void Printer::printTypePtrField(const TypePtrField *s, int tab) {
	printLine(s->mTypePtrVar + "\\" + s->mFieldName + " as " + varTypeToString(s->mFieldType), tab);
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

void Printer::printProgram(const Program *s) {
	printLine("Constants:");
	for (QList<ConstDefinition*>::ConstIterator i = s->mConstants.begin(); i != s->mConstants.end(); i++) {
		printConstDefinition(*i, 1);
	}

	printLine("\n\nTypes:");
	for (QList<TypeDefinition*>::ConstIterator i = s->mTypes.begin(); i != s->mTypes.end(); i++) {
		printTypeDefinition(*i);
	}

	printLine("\n\nGlobals:");
	for (QList<GlobalDefinition*>::ConstIterator i = s->mGlobals.begin(); i != s->mGlobals.end(); i++) {
		printGlobalDefinition(*i, 1);
	}

	printLine("\n\nFunctions:");
	for (QList<FunctionDefinition*>::ConstIterator i = s->mFunctions.begin(); i != s->mFunctions.end(); i++) {
		printFunctionDefinition(*i, 1);
	}

	printLine("\n\nMain block:");
	printBlock(&s->mMainBlock, 1);
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

QString Printer::varTypeToString(const Variable::VarType t) {
	switch (t) {
		case Variable::Default:
			return "Default";
		case Variable::Integer:
			return "Integer";
		case Variable::Float:
			return "Float";
		case Variable::String:
			return "String";
		case Variable::Short:
			return "Short";
		case Variable::Byte:
			return "Byte";
		case Variable::TypePtr:
			return "Type pointer";
		default:
			return "Unknown var type";
	}
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
