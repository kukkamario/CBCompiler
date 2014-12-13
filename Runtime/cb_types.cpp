#include "common.h"
#include "types.h"
#include "error.h"


extern "C" void CB_ConstructType(CB_Type *type, unsigned int size) {
	type->setFirstMember(0);
	type->setLastMember(0);
	type->setSizeOfMember(size);
}

CBEXPORT TypeMember *CB_New(Type *type) {
	return type->createMemberToEnd();
}

CBEXPORT TypeMember *CB_First(Type *type) {
	return type->firstMember();
}

CBEXPORT TypeMember *CB_Last(Type *type) {
	return type->lastMember();
}

CBEXPORT TypeMember *CB_After(TypeMember *member) {
	if (member == 0) {
		//error(U"CBF_CB_TypeMemberAfter: Invalid TypeMember");
		return 0;
	}
	return member->after();
}

CBEXPORT TypeMember *CB_Before(TypeMember *member) {
	if (member == 0) {
		//error(U"CBF_CB_TypeMemberBefore: NULL TypeMember");
		return 0;
	}
	return member->before();
}

CBEXPORT TypeMember *CB_Delete(TypeMember *member) {
	if (member == 0) {
		error(U"CBF_deleteM: NULL TypeMember");
		return 0;
	}
	return member->deleteThis();
}


void CBF_InsertAfter(TypeMember *ptr, TypeMember *target) {
	ptr->type()->moveMemberAfter(ptr, target);
}

void CBF_InsertBefore(TypeMember *ptr, TypeMember *target) {
	ptr->type()->moveMemberBefore(ptr, target);
}



