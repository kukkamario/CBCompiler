#ifndef ABSTRACTSYNTAXTREE_H
#define ABSTRACTSYNTAXTREE_H
#include <QList>
#include <QString>
#include <QObject>
#include <QPair>
#include <QTextStream>
#include "codepoint.h"
#include <cassert>

class QFile;
class Function;
namespace ast {
void printTabs(QTextStream &s, int tabs);

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
			++mChildNodeId;
			assert(validId(mChildNodeId));
			return *this;
		}
		ChildNodeIterator operator++(int) { //i++
			ChildNodeIterator temp = *this;
			++mChildNodeId;
			assert(validId(mChildNodeId));
			return temp;
		}

		ChildNodeIterator &operator--() { //--i
			--mChildNodeId;
			assert(validId(mChildNodeId));
			return *this;
		}
		ChildNodeIterator operator--(int) { //i--
			ChildNodeIterator temp = *this;
			--mChildNodeId;
			assert(validId(mChildNodeId));
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

		template <typename T>
		T *cast() {
			assert(this->type() == T::staticType());
			return static_cast<T*>(this);
		}
		template <typename T>
		const T *cast() const {
			assert(this->type() == T::staticType());
			return static_cast<const T*>(this);
		}

	protected:
		CodePoint mCodePoint;
};

inline Node *ChildNodeIterator::operator*() {
	return mNode->childNode(mChildNodeId);
}

inline bool ChildNodeIterator::validId(int id) {
	//id == mNode->childNodeCount() is the END
	return id >= 0 && id <= mNode->childNodeCount();
}

#define NODE_ACCEPT_VISITOR_PRE_DEF public: void accept(Visitor *visitor);
#define NODE_ACCEPT_VISITOR_DEF(_node_)  void _node_ :: accept(Visitor *visitor) { visitor->visit(this); }
#define NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(_node_) template<> void _node_ ::accept(Visitor *visitor);

class LeafNode : public Node {
	public:
		LeafNode(const CodePoint &cp) : Node(cp) { }
		virtual ~LeafNode() { }

		virtual int childNodeCount() const { return 0; }
		virtual Node *childNode(int) const { assert("LeafNode doesn't have child nodes" && 0); return 0; }
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
		virtual ~Literal() { }
		static Type staticType() { return NT; }
		Type type() const { return staticType(); }
		void setValue(T val) { mValue = val; }
		T value() const { return mValue; }
		void write(QTextStream &s, int tab = 0) {
			printTabs(s, tab);
			s << "Node:" << typeAsString() << " \"" << this->value() << "\"\n";
		}
	protected:
		T mValue;
};

typedef Literal<int, Node::ntInteger> Integer;
typedef Literal<float, Node::ntFloat>  Float;
typedef Literal<QString, Node::ntString> String;

NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(Integer)
NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(Float)
NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(String)





//Identifier
template <Node::Type NT>
class IdentifierT : public LeafNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		IdentifierT(const QString &name, const CodePoint &cp) : LeafNode(cp), mName(name) { }
		virtual ~IdentifierT() { }
		static Type staticType() { return NT; }
		Type type() const { return staticType(); }
		void setName(const QString &name) { mName = name; }
		QString name() const { return mName; }
		void write(QTextStream &s, int tab = 0) {
			printTabs(s, tab);
			s << "Node:" << typeAsString() << " \"" << this->name() << "\"\n";
		}
	protected:
		QString mName;
};

NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(IdentifierT<Node::ntIdentifier>)
NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(IdentifierT<Node::ntLabel>)

typedef IdentifierT<Node::ntLabel> Label;
typedef IdentifierT<Node::ntIdentifier> Identifier;



class Return : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Return(const CodePoint &cp) : Node(cp), mValue(0) { }
		~Return() { delete mValue; }
		static Type staticType() { return ntReturn; }
		Type type() const { return staticType(); }
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
		static Type staticType() { return ntExit; }
		Type type() const { return staticType(); }
};

//ValueTypes:

class DefaultType : public LeafNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		DefaultType(const CodePoint &cp) : LeafNode(cp) {}
		~DefaultType() { }
		static Type staticType() { return ntDefaultType; }
		Type type() const { return staticType(); }
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
		static Type staticType() { return ntBasicType; }
		Type type() const { return staticType(); }
		ValueType valueType() const { return mValueType; }
		void setValueType(ValueType ty) { mValueType = ty; }
	protected:
		ValueType mValueType;
};

class NamedType : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		NamedType(const CodePoint &cp) : Node(cp), mIdentifier(0) { }
		~NamedType() { delete mIdentifier; }
		static Type staticType() { return ntNamedType; }
		Type type() const { return staticType(); }
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
		~ArrayType() { delete mParentType; }
		static Type staticType() { return ntArrayType; }
		Type type() const { return staticType(); }
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
		~Variable() { delete mIdentifier; delete mType; }
		static Type staticType() { return ntVariable; }
		Type type() const { return staticType(); }
		int childNodeCount() const { return 2; }
		Node *childNode(int n) const;

		Identifier *identifier() const { return mIdentifier; }
		void setIdentifier(Identifier *identifier) { mIdentifier = identifier; }
		Node *valueType() const { return mType; }
		void setValueType(Node *type) { mType = type; }
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
		~ExpressionNode() { delete mOperand; }
		static Type staticType() { return ntExpressionNode; }
		Type type() const { return staticType(); }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert(n == 0 && "Invalid child node id"); return mOperand; }

		Op op() const { return mOp; }
		void setOp(Op op) { mOp = op; }
		Node *operand() const { return mOperand; }
		void setOperand(Node *operand) { mOperand = operand; }
		void write(QTextStream &s, int tabs = 0);
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
		~Expression() { delete mFirstOperand; qDeleteAll(mOperations); }
		static Type staticType() { return ntExpression; }
		Type type() const { return staticType(); }
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
		static Type staticType() { return ntList; }
		Type type() const { return staticType(); }
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
		FunctionCall(const CodePoint &cp) : Node (cp), mFunction(0), mParameters(0), mCommand(false) {}
		~FunctionCall() { delete mFunction; delete mParameters; }
		static Type staticType() { return ntFunctionCall; }
		Type type() const { return staticType(); }
		int childNodeCount() const { return 2; }
		Node *childNode(int n) const { assert((n == 0 || n == 1) && "Invalid child node id"); if (n == 0) return mFunction; return mParameters; }
		Node *function() const { return mFunction; }
		void setFunction(Node *f) { mFunction = f; }
		Node *parameters() const { return mParameters; }
		void setParameters(Node *p) { mParameters = p; }
		bool isCommand() const { return mCommand; }
		void setIsCommand(bool t) { mCommand = t; }
	protected:
		Node *mFunction;
		Node *mParameters;
		bool mCommand;
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
		~KeywordFunctionCall() { delete mParameters; }
		static Type staticType() { return ntKeywordFunctionCall; }
		Type type() const { return staticType(); }
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
		~ArraySubscript() { delete mArray; delete mSubscript; }
		static Type staticType() { return ntFunctionCall; }
		Type type() const { return staticType(); }
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
		~DefaultValue() { delete mValueType; }
		static Type staticType() { return ntDefaultValue; }
		Type type() const { return staticType(); }
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
		~Unary() { delete mOperand; }
		static Type staticType() { return ntUnary; }
		Type type() const { return staticType(); }
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
		static Type staticType() { return ntVariableDefinition; }
		Type type() const { return staticType(); }
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
		static Type staticType() { return ntArrayInitialization; }
		Type type() const { return staticType(); }
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
		virtual ~VariableDefinitionStatement() { qDeleteAll(mDefinitions); }
		static Type staticType() { return static_cast<Type>(NT); }
		Type type() const { return staticType(); }
		int childNodeCount() const { return mDefinitions.size(); }
		Node *childNode(int n) const { return mDefinitions.at(n); }

		void appendDefinitions(Node *def) { mDefinitions.append(def); }
		const QList<Node*> &definitions() const { return mDefinitions; }
		void setDefinitions(const QList<Node*> &defs) { mDefinitions = defs; }
	protected:
		QList<Node*> mDefinitions;
};

NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(VariableDefinitionStatement<Node::ntDim>)
NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(VariableDefinitionStatement<Node::ntGlobal>)

typedef VariableDefinitionStatement<Node::ntDim> Dim;
typedef VariableDefinitionStatement<Node::ntGlobal> Global;


class Redim : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Redim(const CodePoint &cp) : Node(cp) { }
		~Redim() { qDeleteAll(mArrayInitializations); }
		static Type staticType() { return ntRedim; }
		Type type() const { return staticType(); }
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
		~TypeDefinition() { delete mIdentifier; qDeleteAll(mFields); }
		static Type staticType() { return ntTypeDefinition; }
		Type type() const { return staticType(); }
		int childNodeCount() const { return 1 + mFields.size(); }
		Node *childNode(int n) const { if (n == 0) return mIdentifier; return mFields.at(n - 1); }

		Identifier *identifier() const { return mIdentifier; }
		void setIdentifier(Identifier *n) { mIdentifier = n; }
		const QList<Node*> &fields() const { return mFields; }
		void setFields(const QList<Node*> &fields) { mFields = fields; }
		void appendField(Node *n) { mFields.append(n); }
	protected:
		Identifier *mIdentifier;
		QList<Node*> mFields;
};

//Blocks
//--------------------------------
class Block : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Block(const CodePoint &start, const CodePoint &end) : BlockNode(start, end) { }
		~Block() { qDeleteAll(mNodes); }
		static Type staticType() { return ntBlock; }
		Type type() const { return staticType(); }
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
		static Type staticType() { return ntIfStatement; }
		Type type() const { return staticType(); }
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
		~SingleConditionBlockStatement() { delete mCondition; delete mBlock; }
		static Type staticType() { return NT; }
		Type type() const { return staticType(); }
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

NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(WhileStatement)
NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(RepeatUntilStatement)



class RepeatForeverStatement : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		RepeatForeverStatement(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mBlock(0) {}
		~RepeatForeverStatement() {}
		static Type staticType() { return ntRepeatForeverStatement; }
		Type type() const { return staticType(); }
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
		static Type staticType() { return ntForToStatement; }
		Type type() const { return staticType(); }
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
		static Type staticType() { return ntForEachStatement; }
		Type type() const { return staticType(); }
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


class SelectCase : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		SelectCase(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mValue(0), mBlock(0) { }
		~SelectCase();
		static Type staticType() { return ntSelectCase; }
		Type type() const { return staticType(); }
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

class SelectStatement : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		SelectStatement(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mVariable(0), mDefault(0) { }
		~SelectStatement();
		static Type staticType() { return ntSelectStatement; }
		Type type() const { return staticType(); }
		int childNodeCount() const { return 1 + mCases.size() + (mDefault != 0 ? 1 : 0); }
		Node *childNode(int n) const;

		Node *variable() const { return mVariable; }
		void setVariable(Node *var) { mVariable = var; }
		const QList<SelectCase*> &cases() const { return mCases; }
		void setCases(const QList<SelectCase*> &cases) { mCases = cases; }
		void appendCase(SelectCase *n) { mCases.append(n); }
		Node *defaultCase() const { return mDefault; }
		void setDefaultCase(Node *n) { mDefault = n; }
	protected:
		Node *mVariable;
		QList<SelectCase*> mCases;
		Node *mDefault;
};


class Const : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Const(const CodePoint &cp) : Node(cp), mVariable(0), mValue(0) { }
		~Const() { delete mVariable; delete mValue; }
		static Type staticType() { return ntConst; }
		Type type() const { return staticType(); }
		int childNodeCount() const { return 2; }
		Node *childNode(int n) const;

		Variable *variable() const { return mVariable; }
		void setVariable(Variable *var) { mVariable = var; }
		Node *value() const { return mValue; }
		void setValue(Node *val) { mValue = val; }
	protected:
		Variable *mVariable;
		Node *mValue;
};

class FunctionDefinition : public BlockNode {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		FunctionDefinition(const CodePoint &start, const CodePoint &end) : BlockNode(start, end), mIdentifier(0), mParameterList(0), mReturnType(0), mBlock(0) { }
		~FunctionDefinition();
		static Type staticType() { return ntFunctionDefinition; }
		Type type() const { return staticType(); }
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
		GotoT(const CodePoint &cp) : Node(cp), mLabel(0) { }
		~GotoT() { delete mLabel; }
		static Type staticType() { return NT; }
		Type type() const { return staticType(); }
		int childNodeCount() const { return 1; }
		Node *childNode(int n) const { assert(n == 0 && "Invalid child node index"); return mLabel; }

		void setLabel(Identifier *n) { mLabel = n; }
		Identifier *label() const { return mLabel; }
	protected:
		Identifier *mLabel;
};

typedef GotoT<Node::ntGoto> Goto;
typedef GotoT<Node::ntGosub> Gosub;

NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(GotoT<Node::ntGoto>)
NODE_ACCEPT_VISITOR_PRE_DEF_TEMPLATE(GotoT<Node::ntGosub>)


class Program : public Node {
		NODE_ACCEPT_VISITOR_PRE_DEF
	public:
		Program() : Node() { }
		~Program();
		static Type staticType() { return ntProgram; }
		Type type() const { return staticType(); }
		int childNodeCount() const { return mFunctionDefinitions.size() + mTypeDefinitions.size() + 1; }
		Node *childNode(int n) const;


		Block *mainBlock() const { return mMainBlock; }
		void setMainBlock(Block *n) { mMainBlock = n; }
		const QList<FunctionDefinition*> &functionDefinitions() const { return mFunctionDefinitions; }
		void setFunctionDefinitions(const QList<FunctionDefinition*> funcDefs) { mFunctionDefinitions = funcDefs; }
		const QList<TypeDefinition*> &typeDefitions() const { return mTypeDefinitions; }
		void setTypeDefinitions(const QList<TypeDefinition*> typeDefs) { mTypeDefinitions = typeDefs; }
	private:
		QList<FunctionDefinition*> mFunctionDefinitions;
		QList<TypeDefinition*> mTypeDefinitions;
		Block *mMainBlock;
};





}

#endif // ABSTRACTSYNTAXTREE_H
