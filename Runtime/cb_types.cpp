#include "common.h"
#include "types.h"
#include "error.h"


extern "C" void CBF_CB_ConstructType(CB_Type *type, unsigned int size) {
	type->setFirstMember(0);
	type->setLastMember(0);
	type->setSizeOfMember(size);
}

CBEXPORT TypeMember *CBF_CB_New(Type *type) {
	return type->createMemberToEnd();
}

CBEXPORT TypeMember *CBF_CB_First(Type *type) {
	return type->firstMember();
}

CBEXPORT TypeMember *CBF_CB_Last(Type *type) {
	return type->lastMember();
}

CBEXPORT TypeMember *CBF_CB_After(TypeMember *member) {
	if (member == 0) {
		error(U"CBF_CB_TypeMemberAfter: Invalid TypeMember");
		return 0;
	}
	return member->after();
}

CBEXPORT TypeMember *CBF_CB_Before(TypeMember *member) {
	if (member == 0) {
		error(U"CBF_CB_TypeMemberBefore: Invalid TypeMember");
		return 0;
	}
	return member->before();
}

CBEXPORT void CBF_deleteM(TypeMember *member) {
	member->deleteThis();
}

