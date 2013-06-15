#ifndef CBFUNCTION_H
#define CBFUNCTION_H
#include "function.h"
#include "constantvalue.h"
#include <QPair>
namespace ast {
struct FunctionDefinition;
struct Node;
typedef QList<Node*> Block;
}
class Scope;

/**
 * @brief The CBFunction class
 */

class CBFunction : public Function {
	public:
		struct Parametre {
				Parametre():mType(0){}
				QString mName;
				ValueType *mType;
				ConstantValue mDefaultValue;
		};

		CBFunction(const QString & name, ValueType *retValue, const QList<Parametre> &params, int line, QFile *file);
		bool generateFunction(const ast::FunctionDefinition *funcDef);
		void setScope(Scope *scope);
		void setBlock(ast::Block *block) {mBlock = block;}
		ast::Block *block() const {return mBlock;}
		Scope *scope() const {return mScope;}
		bool isRuntimeFunction() const {return false;}
		Value call(Builder *builder, const QList<Value> &params);
	private:
		QList<Parametre> mParams;
		Scope *mScope;
		ast::Block *mBlock;
};

#endif // CBFUNCTION_H
