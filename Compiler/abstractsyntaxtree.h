#ifndef ABSTRACTSYNTAXTREE_H
#define ABSTRACTSYNTAXTREE_H
#include <QList>
#include <QString>
#include <QObject>
#include <QPair>
#include <QTextStream>
#include "codepoint.h"

class QFile;
class Function;
namespace ast {
class Visitor;

class Node;
class ChildNodeIterator {
		friend class Node;
	public:
		ChildNodeIterator(const ChildNodeIterator &i) : mNode(i.mNode), mChildNodeId(i.mChildNodeId) {}
		~ChildNodeIterator() {}

		Node *parentNode() const { return mNode; }

		ChildNodeIterator &operator=(const ChildNodeIterator &i) {
			mChildNodeId = i.mChildNodeId;
			mNode = i.mNode;
			return *this;
		}

		ChildNodeIterator &operator+=(int i) {
			assert(validId(mChildNodeId + i));
			mChildNodeId += i;
			return *this;
		}

		ChildNodeIterator &operator-=(int i) {
			assert(validId(mChildNodeId - i));
			mChildNodeId += i;
			return *this;
		}

		ChildNodeIterator &operator++() { //++i
			validId(++mChildNodeId);
			++mChildNodeId;
			return *this;
		}
		ChildNodeIterator operator++(int) { //i++
			validId(++mChildNodeId);
			ChildNodeIterator temp = *this;
			++mChildNodeId;
			return temp;
		}

		ChildNodeIterator &operator--() { //--i
			assert(mChildNodeId != 0);
			--mChildNodeId;
			return *this;
		}
		ChildNodeIterator operator--(int) { //i--
			assert(mChildNodeId != 0);
			ChildNodeIterator temp = *this;
			--mChildNodeId;
			return temp;
		}

		ChildNodeIterator operator+(int i) {
			assert(validId(this->mChildNodeId + i));
			return ChildNodeIterator(mNode, this->mChildNodeId + i);
		}

		ChildNodeIterator operator-(int i) {
			assert(validId(this->mChildNodeId - i));
			return ChildNodeIterator(mNode, this->mChildNodeId - i);
		}

		Node *operator*();


		bool operator ==(const ChildNodeIterator &i) {
			assert(this->mNode == i.mNode);
			return this->mChildNodeId == i.mChildNodeId;
		}

		bool operator !=(const ChildNodeIterator &i) {
			assert(this->mNode == i.mNode);
			return  this->mChildNodeId != i.mChildNodeId;
		}


	private:
		bool validId(int id);

		ChildNodeIterator(Node *n, int childNode) : mNode(n), mChildNodeId(childNode) {}

		Node *mNode;
		int mChildNodeId;
};

class Node {
	public:
		enum Type {
			ntBlock = 0,
			ntInteger,
			ntFloat,
			ntString,
			ntIdentifier,
			ntLabel,
			ntList,
			ntGoto,
			ntGosub,
			ntReturn,
			ntExit,

			ntDefaultType,
			ntBasicType,
			ntNamedType,
			ntArrayType,

			ntExpression,
			ntExpressionNode,
			ntUnary,
			ntArraySubscript,
			ntFunctionCall,
			ntKeywordFunctionCall,
			ntDefaultValue,
			ntVariable,

			ntIfStatement,
			ntWhileStatement,
			ntRepeatForeverStatement,
			ntRepeatUntilStatement,
			ntForToStatement,
			ntForEachStatement,
			ntSelectStatement,
			ntSelectCase,

			ntConst,
			ntDim,
			ntGlobal,
			ntRedim,
			ntVariableDefinition,
			ntArrayInitialization,
			ntFunctionDefinition,
			ntTypeDefinition,

			ntProgram,




			//Last
			ntInvalid
		};
		Node() { }
		Node(const CodePoint &cp) : mCodePoint(cp) { }

		virtual Type type() const = 0;
		virtual ~Node() { }

		virtual const CodePoint &startCodePoint() const { return mCodePoint; }
		virtual const CodePoint &endCodePoint() const { return mCodePoint; }
		virtual const CodePoint &codePoint() const { return mCodePoint; }
		virtual bool isBlock() const { return false; }

		virtual int childNodeCount() const = 0;
		virtual Node *childNode(int n) const = 0;

		virtual void accept(Visitor *visitor) = 0;

		virtual void write(QTextStream &s, int tab = 0);

		ChildNodeIterator childNodesBegin() { return ChildNodeIterator(this, 0); }
		ChildNodeIterator childNodesEnd() { return ChildNodeIterator(this, childNodeCount()); }

		const char *typeAsString() const;
	protected:
		CodePoint mCodePoint;
};

Node *ChildNodeIterator::operator*() {
	return mNode->childNode(mChildNodeId);
}

bool ChildNodeIterator::validId(int id) {
	//id == mNode->childNodeCount() is the END
	return id >= 0 && id <= mNode->childNodeCount();
}

#define NODE_ACCEPT_VISITOR_PRE_DEF public: void accept(Visitor *visitor);
#define NODE_ACCEPT_VISITOR_DEF (_node_)  void _node_##::accept(Visitor *visitor) { visitor->visit(this); }

class LeafNode : public Node {
	public:
		LeafNode(const CodePoint &cp) : Node(cp) { }
		virtual ~LeafNode() { }

		virtual int childNodeCount() const { return 0; }
		virtual Node *childNode(int) { assert("LeafNode doesn't have child nodes" && 0); return 0; }
};

class BlockNode : public Node {
	public:
		BlockNode(const CodePoint &start, const CodePoint &end) : Node(start), mEndCodePoint(end) { }
		virtual ~BlockNode() { }

		const CodePoint &endCodePoint() const { return mEndCodePoint; }
		bool isBlock() const { return true; }

		void setStartCodePoint(const CodePoint &cp) { mCodePoint = cp; }
		void setEndCodePoint(const CodePoint &cp) { mEndCodePoint = cp; }
	protected:
		CodePoint mEndCodePoint;
};


//Literals
//--------------------------------
template <typename T, Node::Type NT>
class Literal : public LeafNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Literal(T value, const CodePoint &cp) : LeafNode(cp), mValue(value) { }
		~Literal() { }
		Type type() const { return NT; }
		void setValue(T val) { mValue = val; }
		T value() const { return mValue; }

	protected:
		T mValue;
};

typedef Literal<int, Node::ntInteger> Integer;
typedef Literal<float, Node::ntFloat> Float;
typedef Literal<QString, Node::ntString> String;


//Identifier
template <Node::Type NT>
class IdentifierT : public LeafNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		IdentifierT(const QString &name, const CodePoint &cp) : LeafNode(cp), mName(name) { }
		virtual ~IdentifierT() { }
		virtual Type type() const { return NT; }
		void setName(const QString &name) { mName = name; }
		QString name() const { return mName; }
	protected:
		QString mName;
};

typedef IdentifierT<Node::ntIdentifier> Identifier;
typedef IdentifierT<Node::ntLabel> Label;


class Return : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Return(const CodePoint &cp) : Node(cp), mValue(0) { }
		~Return() { if (mValue) delete mValue; }
		Type type() const { return ntReturn; }
		int childNodeCount() const { return (mValue != 0) ? 1 : 0; }
		Node *childNode(int n) const { assert(n == 0); return mValue; }

		Node *value() const { return mValue; }
		void setValue(Node *n) { mValue = n; }
	protected:
		Node *mValue;
};

class Exit : public LeafNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Exit(const CodePoint &cp) : LeafNode(cp) { }
		~Exit() { }
		Type type() const { return ntExit; }
};

//ValueTypes:

class DefaultType : public LeafNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		DefaultType(const CodePoint &cp) : LeafNode(cp) {}
		~DefaultType() { }
		Type type() const { return ntDefaultType; }
};

class BasicType : public LeafNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		enum ValueType {
			Integer, //%
			Float,   //#
			String   //$
		};

		BasicType(ValueType ty, const CodePoint &cp) : LeafNode(cp), mValueType(ty) { }
		~BasicType() { }
		Type type() const { return ntBasicType; }
		ValueType valueType() const { return mValueType; }
		void setValueType(ValueType ty) { mValueType = ty; }
	protected:
		ValueType mValueType;
};

class NamedType : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		NamedType(const CodePoint &cp) : Node(cp), mIdentifier(0) { }
		~NamedType() { if (mIdentifier) delete mIdentifier; }
		Type type() const { return ntNamedType; }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert(n == 0 && "Invalid child node index"); return mIdentifier; }

		Identifier *identifier() const { return mIdentifier; }
		void setIdentifier(Identifier *id) { mIdentifier = id; }
	protected:
		Identifier *mIdentifier;
};

class ArrayType : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		ArrayType(const CodePoint &cp) : Node(cp), mParentType(0), mDimensions(0) { }
		~ArrayType() { if (mParentType) delete mParentType; }
		Type type() const { return ntArrayType; }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert(n == 0 && "Invalid child node index"); return mParentType; }

		Node *parentType() const { return mParentType; }
		void setParentType(Node *n) { mParentType = n; }
		int dimensions() const { return mDimensions; }
		void setDimensions(int c) { mDimensions = c; }

	protected:
		Node *mParentType;
		int mDimensions;
};

class Variable : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Variable(const CodePoint &cp) : Node(cp), mIdentifier(0), mType(0) { }
		~Variable() { if (mIdentifier) delete mIdentifier; if (mType) delete mType; }
		Type type() const { return ntVariable; }
		int childNodeCount() const { return 1 + (mType ? 1 : 0); }
		Node *childNode(int n);

		Identifier *identifier() const { return mIdentifier; }
		void setIdentifier(Identifier *identifier) { mIdentifier = identifier; }
		Node *valueType() const { return mType; }
		void setValueType(Node *type) { mType = type; }
		bool hasDefaultType() const { return mType == 0; }
	protected:
		Identifier *mIdentifier;
		Node *mType;
};


class ExpressionNode : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		enum Op {
			opAssign,
			opEqual,
			opNotEqual,
			opGreater,
			opLess,
			opGreaterEqual,
			opLessEqual,
			opAdd,
			opSubtract,
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
			opComma, // ,
			opMember, // .
			opInvalid
		};

		static QString opToString(Op op);

		ExpressionNode(Op op, const CodePoint &cp) : Node(cp), mOp(op), mOperand(0) { }
		~ExpressionNode() { if (mOperand) delete mOperand; }
		Type type() const { return ntExpressionNode; }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert(n == 0 && "Invalid child node id"); return mOperand; }

		Op op() const { return mOp; }
		void setOp(Op op) { mOp = op; }
		Node *operand() const { return mOperand; }
		void setOperand(Node *operand) { mOperand = operand; }
	protected:
		Op mOp;
		Node *mOperand;
};


class Expression : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		enum Associativity {
			LeftToRight,
			RightToLeft
		};

		Expression(Associativity as, const CodePoint &cp) : Node(cp), mFirstOperand(0), mAssociativity(as) { }
		~Expression() { if (mFirstOperand) delete mFirstOperand; qDeleteAll(mOperations); }
		Type type() const { return ntExpression; }
		int childNodeCount() const { return 1 + mOperations.size(); }
		Node *childNode(int n) const { if (n == 0) return mFirstOperand; return mOperations.at(n - 1); }

		Node *firstOperand() const { return mFirstOperand; }
		void setFirstOperand(Node *n) { mFirstOperand = n; }
		void appendOperation(ExpressionNode *n) { mOperations.append(n); }
		const QList<ExpressionNode*> &operations() const { return mOperations; }
		Associativity associativity() const { return mAssociativity; }
	protected:
		Node *mFirstOperand;
		QList<ExpressionNode*> mOperations;
		Associativity mAssociativity;
};

class List : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		List(const CodePoint &cp) : Node(cp) { }
		~List() { qDeleteAll(mItems); }
		Type type() const { return ntList; }
		int childNodeCount() const { return mItems.size(); }
		Node *childNode(int n) const { return mItems.at(n); }
		void appendItem(Node *n) { mItems.append(n); }
		const QList<Node*> &items() const { return mItems; }
		void setItems(const QList<Node*> &items) { mItems = items; }
		void takeAll() { mItems.clear(); }
	protected:
		QList<Node*> mItems;
};

class FunctionCall : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		FunctionCall(const CodePoint &cp) : Node (cp), mFunction(0), mParameters(0) {}
		~FunctionCall() { if (mFunction) delete mFunction; if (mParameters) delete mParameters; }
		Type type() const { return ntFunctionCall; }
		int childNodeCount() const { return 2; }
		Node *childNode(int n) const { assert((n == 0 || n == 1) && "Invalid child node id"); if (n == 0) return mFunction; return mParameters; }
		Node *function() const { return mFunction; }
		void setFunction(Node *f) { mFunction = f; }
		Node *parameters() const { return mParameters; }
		void setParameters(Node *p) { mParameters = p; }
	protected:
		Node *mFunction;
		Node *mParameters;
};

class KeywordFunctionCall : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		enum KeywordFunction {
			New,
			First,
			Last,
			Before,
			After
		};

		KeywordFunctionCall(KeywordFunction type, const CodePoint &cp) : Node (cp), mKeyword(type), mParameters(0) {}
		~KeywordFunctionCall() { if (mParameters) delete mParameters; }
		Type type() const { return ntKeywordFunctionCall; }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert((n == 0) && "Invalid child node id"); return mParameters; }
		Node *parameters() const { return mParameters; }
		void setParameters(Node *p) { mParameters = p; }
	protected:
		KeywordFunction mKeyword;
		Node *mParameters;
};


class ArraySubscript : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		ArraySubscript(const CodePoint &cp) : Node (cp), mArray(0), mSubscript(0) {}
		~ArraySubscript() { if (mArray) delete mArray; if (mSubscript) delete mSubscript; }
		Type type() const { return ntFunctionCall; }
		int childNodeCount() const { return 2; }
		Node *childNode(int n) const { assert((n == 0 || n == 1) && "Invalid child node id"); if (n == 0) return mArray; return mSubscript; }
		Node *array() const { return mArray; }
		void setArray(Node *f) { mArray = f; }
		Node *subscript() const { return mSubscript; }
		void setSubscript(Node *p) { mSubscript = p; }
	protected:
		Node *mArray;
		Node *mSubscript;
};

class DefaultValue : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		DefaultValue(const CodePoint &cp) : Node (cp), mValueType(0) { }
		~DefaultValue() { if (mValueType) delete mValueType; }
		Type type() const { return ntDefaultValue; }
		int childNodeCount() const { return 1; }
		Node * childNode(int n) const { assert(n == 0 && "Invalid child node id"); return mValueType; }

		ast::Node *valueType() const { return mValueType; }
		void setValueType(ast::Node *valTy) { mValueType = valTy; }
	protected:
		ast::Node *mValueType;
};

class Unary : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		enum Op {
			opNot,
			opPositive,
			opNegative,
			opInvalid
		};
		static QString opToString(Op op);

		Unary(Op op, const CodePoint &cp) : Node(cp), mOp(op), mOperand(0) { }
		~Unary() { if (mOperand) delete mOperand; }
		Type type() const { return ntUnary; }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert(n == 0 && "Invalid child node id"); return mOperand; }

		Op op() const { return mOp; }
		Node *operand() const { return mOperand; }
		void setOperand(Node *n) { mOperand = n; }
	protected:
		Op mOp;
		Node *mOperand;
};

class VariableDefinition : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		VariableDefinition(const CodePoint &cp) : Node(cp) { }
		~VariableDefinition();
		Type type() const { return ntVariableDefinition; }
		int childNodeCount() const { return 3; }
		Node *childNode(int n) const;

		ast::Identifier *identifier() const { return mIdentifier; }
		void setIdentifier(ast::Identifier *value) { mIdentifier = value; }
		ast::Node *valueType() const { return mValueType; }
		void setValueType(ast::Node *type) { mValueType = type; }
		ast::Node *value() const { return mValue; }
		void setValue(ast::Node *n) { mValue = n; }
	protected:
		ast::Identifier *mIdentifier;
		ast::Node *mValueType;
		ast::Node *mValue;
};

class ArrayInitialization : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		ArrayInitialization(const CodePoint &cp) : Node(cp), mIdentifier(0), mType(0), mDimensions(0) { }
		~ArrayInitialization();
		Type type() const { return ntArrayInitialization; }
		int childNodeCount() const { return 3; }
		Node *childNode(int n) const;

		Identifier *identifier() const { return mIdentifier; }
		void setIdentifier(Identifier *n) { mIdentifier = n; }
		Node *valueType() const { return mType; }
		void setValueType(Node *type) { mType = type; }
		Node *dimensions() const { return mDimensions; }
		void setDimensions(Node *dim) { mDimensions = dim; }
	protected:
		Identifier *mIdentifier;
		Node *mType;
		Node *mDimensions;
};

template <int NT>
class VariableDefinitionStatement : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		VariableDefinitionStatement(const CodePoint &cp) : Node(cp) { }
		~VariableDefinitionStatement() { qDeleteAll(mDefinitions); }
		Type type() const { return NT; }
		int childNodeCount() const { return mDefinitions.size(); }
		Node *childNode(int n) const { return mDefinitions.at(n); }

		void appendDefinitions(Node *def) { mDefinitions.append(def); }
		const QList<Node*> definitions() const { return mDefinitions; }
		void setDefinitions(const QList<Node*> &defs) { mDefinitions = defs; }
	protected:
		QList<Node*> mDefinitions;
};
typedef VariableDefinitionStatement<Node::ntDim> Dim;
typedef VariableDefinitionStatement<Node::ntGlobal> Global;

class Redim : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Redim(const CodePoint &cp) : Node(cp) { }
		~Redim() { qDeleteAll(mArrayInitializations); }
		Type type() const { return ntRedim; }
		int childNodeCount() const { return mArrayInitializations.size(); }
		Node *childNode(int i) const { return mArrayInitializations.at(i); }

		QList<ArrayInitialization*> arrayInitialization() const { return mArrayInitializations; }
		void setArrayInializations(const QList<ArrayInitialization*> &inits) { mArrayInitializations = inits; }

	private:
		QList<ArrayInitialization*> mArrayInitializations;
};


class TypeDefinition : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		TypeDefinition(const CodePoint &start, const CodePoint &end) : BlockNode(start, end) { }
		~TypeDefinition() { if (mIdentifier) delete mIdentifier; qDeleteAll(mFields); }
		Type type() const { return ntTypeDefinition; }
		int childNodeCount() const { return 1 + mFields.size(); }
		Node *childNode(int n) const { if (n == 0) return mIdentifier; return mFields.at(n - 1); }

		Node *identifier() const { return mIdentifier; }
		void setIdentifier(Node *n) { mIdentifier = n; }
		const QList<Node*> &fields() const { return mFields; }
		void setFields(const QList<Node*> &fields) { mFields = fields; }
		void appendField(Node *n) { mFields.append(n); }
	protected:
		Node *mIdentifier;
		QList<Node*> mFields;
};

//Blocks
//--------------------------------
class Block : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Block(const CodePoint &start, const CodePoint &end) : BlockNode(start, end) { }
		~Block() { qDeleteAll(mNodes); }
		Type type() const { return ntBlock; }
		int childNodeCount() const { return mNodes.size(); }
		Node *childNode(int n) const { return mNodes.at(n); }

		void appendNode(Node *n) { mNodes.append(n); }
		const QList<Node *> &childNodes() const { return mNodes; }
		void setChildNodes(const QList<Node*> &nodes) { mNodes = nodes; }
	protected:
		QList<Node *> mNodes;
};

class IfStatement : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		IfStatement(const CodePoint &start, const CodePoint &end) :
			BlockNode(start, end), mCondition(0), mBlock(0), mElse(0) { }
		~IfStatement();
		Type type() const { return ntIfStatement; }
		int childNodeCount() const { return 2 + ((mElse != 0) ? 1 : 0); }
		Node *childNode(int n) const;

		Node *condition() const { return mCondition; }
		void setCondition(Node *n) { mCondition = n; }
		Node *block() const { return mBlock; }
		void setBlock(Node *n) { mBlock = n; }
		Node *elseBlock() const { return mElse; }
		void setElseBlock(Node *n) { mElse = n; }
	protected:
		Node *mCondition;
		Node *mBlock;
		Node *mElse;
};

template <Node::Type NT, bool CondBeforeBlock>
class SingleConditionBlockStatement : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		SingleConditionBlockStatement(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mCondition(0), mBlock(0) { }
		~SingleConditionBlockStatement() { if (mCondition) delete mCondition; if (mBlock) delete mBlock; }
		Type type() const { return NT; }
		int childNodeCount() const { return 2; }
		Node *childNode(int n) const {assert((n == 0 || n == 1) && "Invalid child node id"); return (n == CondBeforeBlock) ? mBlock : mCondition;}

		Node *condition() const { return mCondition; }
		void setCondition(Node *n) { mCondition = n; }
		Node *block() const { return mBlock; }
		void setBlock(Node *n) { mBlock = n; }
	protected:
		Node *mCondition;
		Node *mBlock;
};

typedef SingleConditionBlockStatement<Node::ntWhileStatement, true> WhileStatement;
typedef SingleConditionBlockStatement<Node::ntRepeatUntilStatement, false> RepeatUntilStatement;

class RepeatForeverStatement : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		RepeatForeverStatement(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mBlock(0) {}
		~RepeatForeverStatement() {}
		Type type() const { return ntRepeatUntilStatement; }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert(n == 0 && "Invalid child node id"); return mBlock; }

		Node *block() const { return mBlock; }
		void setBlock(Node *block) { mBlock = block; }
	protected:
		Node *mBlock;
};

class ForToStatement : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		ForToStatement(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mFrom(0), mTo(0), mStep(0), mBlock(0) { }
		~ForToStatement();
		Type type() const { return ntForToStatement; }
		int childNodeCount() const { return 3 + (mStep != 0 ? 1 : 0); }
		Node *childNode(int n) const;

		Node *from() const { return mFrom; }
		void setFrom(Node *n) { mFrom = n; }
		Node *to() const { return mTo; }
		void setTo(Node *n) { mTo = n; }
		Node *step() const { return mStep; }
		void setStep(Node *n) { mStep = n; }
		Node *block() const { return mBlock; }
		void setBlock(Node *n) { mBlock = n; }
	protected:
		Node *mFrom;
		Node *mTo;
		Node *mStep;
		Node *mBlock;
};

class ForEachStatement : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		ForEachStatement(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mVariable(0), mContainer(0), mBlock(0) { }
		~ForEachStatement();
		Type type() const { return ntForEachStatement; }
		int childNodeCount() const { return 3; }
		Node *childNode(int n) const;

		Node *variable() const { return mVariable; }
		void setVariable(Node *n) { mVariable = n; }
		Node *container() const { return mContainer; }
		void setContainer(Node *n) { mContainer = n; }
		Node *block() const { return mBlock; }
		void setBlock(Node *n) { mBlock = n; }
	protected:
		Node *mVariable;
		Node *mContainer;
		Node *mBlock;
};

class SelectStatement : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		SelectStatement(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mVariable(0), mDefault(0) { }
		~SelectStatement();
		Type type() const { return ntSelectStatement; }
		int childNodeCount() { return 1 + mCases.size() + (mDefault != 0 ? 1 : 0); }
		Node *childNode(int n) const;

		Node *variable() const { return mVariable; }
		void setVariable(Node *var) { mVariable = var; }
		const QList<Node*> cases() const { return mCases; }
		void setCases(const QList<Node*> &cases) { mCases = cases; }
		void appendCase(Node *n) { mCases.append(n); }
		Node *defaultCase() const { return mDefault; }
		void setDefaultCase(Node *n) { mDefault = n; }
	protected:
		Node *mVariable;
		QList<Node*> mCases;
		Node *mDefault;
};

class SelectCase : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		SelectCase(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mValue(0), mBlock(0) { }
		~SelectCase();
		Type type() const { return ntSelectCase; }
		int childNodeCount() const { return 2; }
		Node *childNode(int n) const { assert("Invalid child node id" && n >= 0 && n < 2); return n == 0 ? mValue : mBlock; }

		Node *value() const { return mValue; }
		void setValue(Node *n) { mValue = n; }
		Node *block() const { return mBlock; }
		void setBlock(Node *n) { mBlock = n; }
	protected:
		Node *mValue;
		Node *mBlock;
};

class Const : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Const(const CodePoint &cp) : Node(cp), mIdentifier(0), mValue(0) { }
		~Const() { if (mIdentifier) delete mIdentifier; if (mValue) delete mValue; }
		Type type() const { return ntConst; }
		int childNodeCount() const { return 2; }
		Node *childNode(int n) const;

		Node *identifier() const { return mIdentifier; }
		void setIdentifier(Node *var) { mIdentifier = var; }
		Node *value() const { return mValue; }
		void setValue(Node *val) { mValue = val; }
	protected:
		Node *mIdentifier;
		Node *mValue;
};

class FunctionDefinition : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		FunctionDefinition(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mIdentifier(0), mParameterList(0), mReturnType(0), mBlock(0) { }
		~FunctionDefinition();
		Type type() const { return ntFunctionDefinition; }
		int childNodeCount() const { return 4; }
		Node *childNode(int n) const;

		Identifier *identifier() const { return mIdentifier; }
		void setIdentifier(Identifier *id) { mIdentifier = id; }
		Node *parameterList() const { return mParameterList; }
		void setParameterList(Node *n) { mParameterList = n; }
		Node *returnType() const { return mReturnType; }
		void setReturnType(Node *n) { mReturnType = n; }
		Node *block() const { return mBlock; }
		void setBlock(Node *n) { mBlock = n; }
	private:
		Identifier *mIdentifier;
		Node *mParameterList;
		Node *mReturnType;
		Node *mBlock;
};


template <Node::Type NT>
class GotoT : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		GotoT(const QString &label, const CodePoint &cp) : Node(cp), mLabel(label) { }
		~GotoT() { if (mLabel) delete mLabel; }
		Type type() const { return NT; }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert(n == 0 && "Invalid child node index"); return mLabel; }

		void setLabel(Node *n) { mLabel = label; }
		Node *label() const { return mLabel; }
	protected:
		Node *mLabel;
};

typedef GotoT<Node::ntGoto> Goto;
typedef GotoT<Node::ntGosub> Gosub;

class Program : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Program() : Node() { }
		~Program();
		Type type() const { return ntProgram; }
		int childNodeCount() const { return mFunctionDefinitions.size() + mTypeDefinitions.size() + 1; }
		Node *childNode(int n) const;


		Node *mainBlock() const { return mMainBlock; }
		void setMainBlock(Node *n) { mMainBlock = n; }
		const QList<FunctionDefinition*> &functionDefinitions() const { return mFunctionDefinitions; }
		void setFunctionDefinitions(const QList<FunctionDefinition*> funcDefs) { mFunctionDefinitions = funcDefs; }
		const QList<TypeDefinition*> &typeDefitions() const { return mTypeDefinitions; }
		void setTypeDefinitions(const QList<TypeDefinition*> typeDefs) { mTypeDefinitions = typeDefs; }
	private:
		QList<FunctionDefinition*> mFunctionDefinitions;
		QList<TypeDefinition*> mTypeDefinitions;
		Node *mMainBlock;
};





}

#endif // ABSTRACTSYNTAXTREE_H
