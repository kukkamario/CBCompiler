#include "types.h"

CB_TypeMember *CB_Type::createMemberToEnd() {
	char *memberData = new char[mSizeOfMember];
	CB_TypeMember *member = reinterpret_cast<CB_TypeMember*>(memberData);
	member->mType = this;
	memset(member->mData, 0, sizeOfMemberData());
	insertLast(member);
	return member;
}


void CB_Type::insertLast(CB_TypeMember *member) {
	if (mLast) {
		mLast->mAfter = member;
	} else if (!mFirst) {
		mFirst = member;
	}
	member->mBefore = mLast;
	member->mAfter = 0;
	mLast = member;
}
