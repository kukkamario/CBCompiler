#include "typechecker.h"
#include "scope.h"

TypeChecker::TypeChecker() :
	QObject() {
}

void TypeChecker::visit(ast::Expression *expr) {

}

void TypeChecker::visit(ast::ArrayInitialization *arrayInit)
{

}

void TypeChecker::visit(ast::Goto *g)
{

}

void TypeChecker::visit(ast::Gosub *g)
{

}

void TypeChecker::visit(ast::Dim *d)
{

}

void TypeChecker::visit(ast::Global *g)
{

}
