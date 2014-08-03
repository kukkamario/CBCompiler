#include "types.h"
#include "error.h"
#include <cstdio>

void CB_Type::construct(unsigned int sizeOfMember) {
	mFirst = 0;
	mLast = 0;
	mSizeOfMember = sizeOfMember;
	mDummyFirst.mAfter = 0;
	mDummyFirst.mType = this;
}

CB_TypeMember *CB_Type::createMemberToEnd() {
	char *memberData = new char[mSizeOfMember];
	CB_TypeMember *member = reinterpret_cast<CB_TypeMember*>(memberData);
	member->mType = this;
	memset(member->mData, 0, sizeOfMemberData());
	insertLast(member);
	return member;
}

CB_TypeMember *CB_Type::deleteMember(CB_TypeMember *m) {
	assert(m->type() == this);
	assert(m != &mDummyFirst && "Only member of a type is deleted twice");
	CB_TypeMember *ret = 0;
	if (m->after()) {
		m->after()->mBefore = m->before();
		if (m->before()) {
			m->before()->mAfter = m->after();
			ret = m->before();
		}
		else { //m is the first member
			setFirstMember(m->after());
			ret = &mDummyFirst;
		}
	}
	else { //m is the last member
		if (m->before()) {
			m->before()->mAfter = 0;
			setLastMember(m->before());
			ret = m->before();
		}
		else { //m is the only member
			setFirstMember(0);
			setLastMember(0);
			ret = &mDummyFirst;
		}
	}

	delete [] reinterpret_cast<char*>(m);
	return ret;
}

void CB_Type::setFirstMember(CB_TypeMember *m) {
	mFirst = m;
	mDummyFirst.mAfter = m;
}

void CB_Type::setLastMember(CB_TypeMember *m) {
	 mLast = m;
}


void CB_Type::insertLast(CB_TypeMember *member) {
	if (mLast) {
		mLast->mAfter = member;
	} else if (!mFirst) {
		setFirstMember(member);
	}
	member->mBefore = mLast;
	member->mAfter = 0;
	setLastMember(member);
}


CB_TypeMember *CB_TypeMember::deleteThis() {
	return mType->deleteMember(this);
}
