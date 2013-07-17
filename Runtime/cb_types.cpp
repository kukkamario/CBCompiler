#include "common.h"
#include "types.h"
#include "error.h"
static int sTypeIdCounter = 1;
CBEXPORT void CBF_CB_ConstructType(Type *type, unsigned int size) {
	type->mFirst = 0;
	type->mLast = 0;
	type->mId = sTypeIdCounter++;
	type->mSizeOfMember = size;
}

CBEXPORT TypeMember *CBF_newT(Type *type) {
	return type->createMemberToEnd();
}

CBEXPORT TypeMember *CBF_afterM(TypeMember *member) {
	if (member == 0) {
		error(U"CBF_CB_TypeMemberAfter: Invalid TypeMember");
		return 0;
	}
	return member->after();
}

CBEXPORT TypeMember *CBF_beforeM(TypeMember *member) {
	if (member == 0) {
		error(U"CBF_CB_TypeMemberBefore: Invalid TypeMember");
		return 0;
	}
	return member->before();
}

CBEXPORT TypeMember *CBF_firstT(Type *type) {
	return type->firstMember();
}

CBEXPORT TypeMember *CBF_lastT(Type *type) {
	return type->lastMember();
}

