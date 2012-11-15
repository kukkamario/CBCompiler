#ifndef ABSTRACTSYNTAXTREE_H
#define ABSTRACTSYNTAXTREE_H
#include <QList>
#include <QString>
#include <QObject>
class QFile;
namespace ast {
enum Operator {
	opEqual,
	opNotEqual,
	opGreater,
	opLess,
	opGreaterEqual,
	opLessEqual,
	opPlus,
	opMinus,
	opMultiply,
	opPower,
	opMod,
	opShl,
	opShr,
	opSar,
	opDivide,
	opAnd,
	opOr,
	opXor,
	opNot,
	opInvalid
};
QString operatorToString(Operator op);

struct Node {
	enum Type {
		ntBlock,
		ntFunctionDefinition,
		ntTypeDefinition,

		//Statements
		ntIfStatement,
		ntWhileStatement,
		ntForToStatement,
		ntForEachStatement,
		ntRepeatForeverStatement,
		ntRepeatUntilStatement,

		ntExpression,

		ntCommandCall,
		ntArrayDefinition, //Dim
		ntVariableDefinition, //Dim
		ntGlobalDefinition, //Global
		ntAssignmentExpression, //a = (...)
		ntUnary,
		ntReturn,

		//Operands
		ntTypePtrField,	// typePtr\field
		ntVariable,		// var
		ntInteger,		// 13213
		ntFloat,		// 213.02
		ntString,		// "string"
		ntFunctionCallOrArraySubscript,
		ntNew			// new (type)
	};
	virtual Type type() const = 0;
};

typedef QList<Node*> Block;

struct Variable : Node {
		enum VarType {
			Invalid,
			Default,
			Integer,
			Float,
			String,
			Short,
			Byte,
			TypePtr
		};

		Type type() const {return ntVariable;}
		VarType varType() const { return mVarType;}

		VarType mVarType;
		QString mName;

		QString mTypeName; //If VarType == TypePtr
};

struct Integer : Node {
		Type type() const {return ntInteger;}
		int mValue;
};

struct Return : Node {
		Type type() const {return ntReturn; }
		Node *mValue;
};

struct New : Node {
		Type type() const {return ntNew;}
		QString mTypeName;
};

struct Float : Node {
		Type type() const {return ntFloat;}
		float mValue;
};

struct VariableDefinition : Node {
		Type type() const {return ntVariableDefinition;}
		QList<Variable*> mDefinitions;
};

struct TypeDefinition : Node {
		Type type() const {return ntTypeDefinition;}
		QString mName;
		QList<VariableDefinition> mFields;
};

struct String : Node {
		Type type() const {return ntString;}
		QString mValue;
};

struct Unary : Node {
		Type type() const { return ntUnary;}
		Operator mOperator;
		Node *mOperand;
};

struct FunctionParametreDefinition {
		Variable mParam;
		Node *mDefaultValue; //Null if not specified
};

struct FunctionDefinition : Node{
		Type type() const {return ntFunctionDefinition;}
		Variable::VarType mRetType;
		QString mName;
		QList<FunctionParametreDefinition> mParams;
		Block mBlock;
};

struct TypePtrField : Node {
		Type type() const { return ntTypePtrField;}
		QString mTypePtrVar;
		QString mFieldName;
		Variable::VarType mFieldType;
};

struct FunctionCallOrArraySubscript : Node {
		Type type() const {return ntFunctionCallOrArraySubscript;}
		QString mName;
		QList<Node*> mParams;
};

struct CommandCall: Node {
		Type type() const{return ntCommandCall;}
		QString mName;
		QList<Node*> mParams;
};



struct GlobalDefinition : Node {
		Type type() const {return ntGlobalDefinition;}
		QList<Variable*> mDefinitions;
};

struct ArrayDefinition : Node {
		Type type() const{ return ntArrayDefinition;}
		QString mName;
		Variable::Type mType;
		QList<Node*> mDimensions;
};

struct Operation {
		Operation(Operator opp, Node *opr) : mOperator(opp), mOperand(opr) {}
		Operator mOperator;
		Node *mOperand;
};


struct Expression :Node{
		Type type() const {return ntExpression;}
		Node *mFirst;
		QList<Operation> mRest;
};

struct AssignmentExpression : Node{
		Type type() const{return ntAssignmentExpression;}
		Variable *mVariable;
		Node *mExpression;
};

struct IfStatement : Node {
		IfStatement():mCondition(0), mElse(0) {}
		Type type() const {return ntIfStatement;}
		Node *mCondition;
		Block mIfTrue;
		Block mElse; //Null if not defined
};

struct ForToStatement : Node {
		ForToStatement();
		Type type() const{return ntForToStatement;}
		QString mVarName;
		Variable::VarType mVarType;
		Node *mFrom;
		Node *mTo;
		Node *mStep;
		Block mBlock;
};

struct ForEachStatement : Node {
		Type type() const{return ntForEachStatement;}
		QString mVarName; //TypePtr
		QString mTypeName;
		Block mBlock;
};

struct WhileStatement : Node {
		WhileStatement(): mCondition(0) {}
		Type type() const{return ntWhileStatement;}
		Node *mCondition;
		Block mBlock;
};
struct RepeatForeverStatement : Node {
		Type type() const {return ntRepeatForeverStatement;}
		Block mBlock;
};

struct RepeatUntilStatement : Node {
		RepeatUntilStatement(): mCondition(0) {}
		Type type()const {return ntRepeatUntilStatement;}
		Block mBlock;
		Node *mCondition;
};



struct Program {
		QList<FunctionDefinition*> mFunctionDefinitions;
		QList<GlobalDefinition*> mGlobals;
		Block mMainBlock;
};

class Printer : public QObject{
		Q_OBJECT
	public:
		Printer(QObject *parent = 0) : QObject(parent), mDestFile(0) {}
		void printLine(const QString &txt, int tab = 0);

		void printNode(const Node *s, int tab = 0);
		void printBlock(const Block *s, int tab = 0);
		void printTypeDefinition(const TypeDefinition *s, int tab = 0);

		void printIfStatement(const IfStatement *s, int tab = 0);
		void printWhileStatement(const WhileStatement *s, int tab = 0);
		void printForToStatement(const ForToStatement *s, int tab = 0);
		void printForEachStatement(const ForEachStatement *s, int tab = 0);
		void printRepeatForeverStatement(const RepeatForeverStatement *s, int tab = 0);
		void printRepeatUntilStatement(const RepeatUntilStatement *s, int tab = 0);

		void printExpression(const Expression *s, int tab = 0);

		void printCommandCall(const CommandCall *s, int tab = 0);
		void printArrayDefinition(const ArrayDefinition *s, int tab = 0);
		void printVariableDefinition(const VariableDefinition *s, int tab = 0);
		void printGlobalDefinition(const GlobalDefinition *s, int tab = 0);
		void printAssignmentExpression(const AssignmentExpression *s, int tab = 0);
		void printUnary(const Unary *s, int tab = 0);
		void printReturn(const Return *s, int tab = 0);

		void printTypePtrField(const TypePtrField *s, int tab = 0);
		void printVariable(const Variable *s, int tab = 0);
		void printInteger(const Integer *s,  int tab = 0);
		void printFloat(const Float *s, int tab = 0);
		void printString(const String *s, int tab = 0);
		void printFunctionCallOrArraySubscript(const FunctionCallOrArraySubscript *s, int tab = 0);
		void printNew(const New *s, int tab = 0);

		bool printToFile(const QString &file);
	private:
		QFile *mDestFile;

};



}

#endif // ABSTRACTSYNTAXTREE_H
