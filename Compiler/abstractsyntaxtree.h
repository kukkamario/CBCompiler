#ifndef ABSTRACTSYNTAXTREE_H
#define ABSTRACTSYNTAXTREE_H
#include <QList>
#include <QString>
#include <QObject>
#include <QPair>
class QFile;
class Function;
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
		ntLabel,

		//Statements
		ntIfStatement,
		ntWhileStatement,
		ntForToStatement,
		ntForEachStatement,
		ntRepeatForeverStatement,
		ntRepeatUntilStatement,
		ntGoto,
		ntGosub,
		ntRedim,


		ntExpression,

		ntCommandCall,
		ntArrayDefinition, //Dim
		ntVariableDefinition, //Dim
		ntGlobalDefinition, //Global
		ntConstDefinition,//Const
		ntAssignmentExpression, //a = (...)
		ntArraySubscriptAssignmentExpression, // array( 123, 42) = ...
		ntCommandCallOrArraySubscriptAssignmentExpression, // array (21 + 23) = 123, command (x +2 ) = 23
		ntUnary,
		ntReturn,
		ntExit,
		ntSelectStatement,
		ntSpecialFunctionCall, //New, First, Last, Before, After

		//Operands
		ntTypePtrField,	// typePtr\field
		ntVariable,		// var
		ntInteger,		// 13213
		ntFloat,		// 213.02
		ntString,		// "string"
		ntFunctionCallOrArraySubscript
	};
	virtual Type type() const = 0;
	virtual ~Node() { }
};

struct Block : public QList<Node*> {
		Block() {}
		//~Block() { qDeleteAll(*this); }
};

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
		QFile *mFile;
		int mLine;
};

struct Float : Node {
		Type type() const {return ntFloat;}
		float mValue;
};

struct VariableDefinition : Node {
		Type type() const {return ntVariableDefinition;}
		~VariableDefinition() { qDeleteAll(mDefinitions);}
		QList<Variable*> mDefinitions;
		int mLine;
		QFile *mFile;
};

struct TypeDefinition : Node {
		Type type() const {return ntTypeDefinition;}
		~TypeDefinition();
		QString mName;
		QList<QPair<int, Variable*> > mFields;
		int mLine;
		QFile *mFile;
};

struct String : Node {
		Type type() const {return ntString;}
		QString mValue;
};

struct Goto : Node {
		Type type() const {return ntGoto;}
		QString mLabel;
		int mLine;
		QFile *mFile;
};
struct Gosub : Node {
		Type type() const {return ntGosub;}
		QString mLabel;
		int mLine;
		QFile *mFile;
};

struct Unary : Node {
		Type type() const { return ntUnary;}
		~Unary() { delete mOperand; }
		Operator mOperator;
		Node *mOperand;
};

struct FunctionParametreDefinition {
		Variable mVariable;
		Node *mDefaultValue; //Null if not specified
};

struct Exit : Node {
		Type type()const{return ntExit;}
		int mLine;
		QFile *mFile;
};

struct FunctionDefinition : Node{
		Type type() const {return ntFunctionDefinition;}
		Variable::VarType mRetType;
		QString mName;
		QList<FunctionParametreDefinition> mParams;
		Block mBlock;
		int mLine;
		QFile *mFile;
};

struct TypePtrField : Node {
		Type type() const { return ntTypePtrField;}
		QString mTypePtrVar;
		QString mFieldName;
		Variable::VarType mFieldType;
};

struct FunctionCallOrArraySubscript : Node {
		~FunctionCallOrArraySubscript() { qDeleteAll(mParams); }
		Type type() const {return ntFunctionCallOrArraySubscript;}
		QString mName;
		QList<Node*> mParams;
		int mLine;
		QFile *mFile;
};

struct SpecialFunctionCall : Node {
		enum SpecialFunction {
			New,
			First,
			Last,
			Before,
			After
		};

		Type type() const { return ntSpecialFunctionCall; }
		SpecialFunction mSpecialFunction;
		Node *mParam;
		int mLine;
		QFile *mFile;
};

struct CommandCallOrArraySubscriptAssignmentExpression : Node {
		~CommandCallOrArraySubscriptAssignmentExpression() { if (mIndexOrExpressionInParentheses) delete mIndexOrExpressionInParentheses; if (mEqualTo) delete mEqualTo; }
		Type type() const { return ntCommandCallOrArraySubscriptAssignmentExpression; }
		QString mName;
		Node *mIndexOrExpressionInParentheses;
		Node *mEqualTo;
		int mLine;
		QFile *mFile;
};

struct CommandCall: Node {
		~CommandCall() { qDeleteAll(mParams); }
		Type type() const{return ntCommandCall;}
		QString mName;
		QList<Node*> mParams;
		int mLine;
		QFile *mFile;
};

struct ConstDefinition: Node {
		~ConstDefinition() { delete mValue; }
		Type type() const{return ntConstDefinition;}
		QString mName;
		Variable::VarType mVarType;
		ast::Node *mValue;
		int mLine;
		QFile *mFile;
};

struct Label: Node {
		Type type() const{return ntLabel;}
		QString mName;
		int mLine;
		QFile *mFile;
};


struct GlobalDefinition : Node {
		~GlobalDefinition() { qDeleteAll(mDefinitions); }
		Type type() const {return ntGlobalDefinition;}
		QList<Variable*> mDefinitions;
		int mLine;
		QFile *mFile;
};

struct ArrayDefinition : Node {
		~ArrayDefinition() { qDeleteAll(mDimensions); }
		Type type() const{ return ntArrayDefinition;}
		QString mName;
		Variable::VarType mType;
		QList<Node*> mDimensions;
		int mLine;
		QFile *mFile;
};

struct Operation {
		Operation(Operator opp, Node *opr) : mOperator(opp), mOperand(opr) {}
		//~Operation() { delete mOperand; }
		Operator mOperator;
		Node *mOperand;
};


struct Expression :Node{
		~Expression() { delete mFirst; }
		Type type() const {return ntExpression;}
		Node *mFirst;
		QList<Operation> mRest;
};

struct AssignmentExpression : Node{
		~AssignmentExpression() { delete mVariable; delete mExpression; }
		Type type() const{return ntAssignmentExpression;}
		Node *mVariable;
		Node *mExpression;
		int mLine;
		QFile *mFile;
};

struct ArraySubscriptAssignmentExpression : Node {
		~ArraySubscriptAssignmentExpression() { qDeleteAll(mSubscripts); delete mValue; }
		Type type() const {return ntArraySubscriptAssignmentExpression;}
		QString mArrayName;
		QList<Node*> mSubscripts;
		Node * mValue;

		QFile *mFile;
		int mLine;
};

struct IfStatement : Node {
		IfStatement():mCondition(0) {}
		~IfStatement() { delete mCondition; }
		Type type() const {return ntIfStatement;}
		Node *mCondition;
		Block mIfTrue;
		Block mElse; //Null if not defined

		QFile *mFile;
		int mLine;
};

struct Case {
		~Case() { delete mCase; }
		Node *mCase;
		Block mBlock;
		int mLine;
		QFile *mFile;
};

struct SelectStatement : Node {
		~SelectStatement() { delete mVariable; }
		Type type() const {return ntSelectStatement;}
		Variable *mVariable;
		QList<Case> mCases;
		Block mDefault;
		QFile *mFile;
		int mStartLine;
		int mEndLine;
};

struct ForToStatement : Node {
		ForToStatement() : mFrom(0), mTo(0), mStep(0), mFile(0){}
		~ForToStatement() { delete mFrom; delete mTo; if(mStep) delete mStep; }
		Type type() const{return ntForToStatement;}
		QString mVarName;
		Variable::VarType mVarType;
		Node *mFrom;
		Node *mTo;
		Node *mStep;
		Block mBlock;

		QFile *mFile;
		int mStartLine;
		int mEndLine;
};

struct ForEachStatement : Node {
		Type type() const{return ntForEachStatement;}
		QString mVarName; //TypePtr
		QString mTypeName;
		Block mBlock;

		QFile *mFile;
		int mStartLine;
		int mEndLine;
};

struct WhileStatement : Node {
		WhileStatement(): mCondition(0) {}
		~WhileStatement() { delete mCondition; }
		Type type() const{return ntWhileStatement;}
		Node *mCondition;
		Block mBlock;

		QFile *mFile;
		int mStartLine;
		int mEndLine;
};
struct RepeatForeverStatement : Node {
		Type type() const {return ntRepeatForeverStatement;}
		Block mBlock;
		QFile *mFile;
		int mStartLine;
		int mEndLine;
};

struct RepeatUntilStatement : Node {
		RepeatUntilStatement(): mCondition(0) {}
		~RepeatUntilStatement() { delete mCondition; }
		Type type()const {return ntRepeatUntilStatement;}
		Block mBlock;
		Node *mCondition;
		QFile *mFile;
		int mStartLine;
		int mEndLine;
};

struct Redim : Node {
		~Redim() { qDeleteAll(mDimensions); }
		Type type() const{return ntRedim;}
		QString mName;
		Variable::VarType mType;
		QList<Node*> mDimensions;
		int mLine;
		QFile *mFile;
};



struct Program {
		~Program() { qDeleteAll(mTypes); qDeleteAll(mConstants); qDeleteAll(mFunctions); qDeleteAll(mGlobals); }
		QList<TypeDefinition*> mTypes;
		QList<ConstDefinition*> mConstants;
		QList<FunctionDefinition*> mFunctions;
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
		void printSelectStatement(const SelectStatement *s, int tab = 0);

		void printGoto(const Goto *s, int tab = 0);
		void printGosub(const Gosub *s, int tab = 0);
		void printLabel(const Label *s, int tab = 0);

		void printExpression(const Expression *s, int tab = 0);

		void printFunctionDefinition(const FunctionDefinition *s, int tab = 0);

		void printCommandCall(const CommandCall *s, int tab = 0);
		void printArrayDefinition(const ArrayDefinition *s, int tab = 0);
		void printVariableDefinition(const VariableDefinition *s, int tab = 0);
		void printConstDefinition(const ConstDefinition *s, int tab = 0);
		void printGlobalDefinition(const GlobalDefinition *s, int tab = 0);
		void printAssignmentExpression(const AssignmentExpression *s, int tab = 0);
		void printArraySubscriptAssignmentExpression(const ArraySubscriptAssignmentExpression *s, int tab = 0);
		void printCommandCallOrArraySubscriptAssignmentExpression(const CommandCallOrArraySubscriptAssignmentExpression *s, int tab = 0);
		void printUnary(const Unary *s, int tab = 0);
		void printReturn(const Return *s, int tab = 0);
		void printExit(const Exit *s, int tab = 0);
		void printSpecialFunctionCall(const SpecialFunctionCall *s, int tab = 0);


		void printTypePtrField(const TypePtrField *s, int tab = 0);
		void printVariable(const Variable *s, int tab = 0);
		void printInteger(const Integer *s,  int tab = 0);
		void printFloat(const Float *s, int tab = 0);
		void printString(const String *s, int tab = 0);
		void printFunctionCallOrArraySubscript(const FunctionCallOrArraySubscript *s, int tab = 0);

		void printProgram(const Program *s);

		bool printToFile(const QString &file);

		QString varTypeToString(const Variable::VarType t);
	private:
		QFile *mDestFile;

};



}

#endif // ABSTRACTSYNTAXTREE_H
