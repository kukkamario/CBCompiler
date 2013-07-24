#include "types.h"

CB_TypeMember *CB_Type::createMemberToEnd() {
	char *memberData = new char[mSizeOfMember];
	CB_TypeMember *member = reinterpret_cast<CB_TypeMember*>(memberData);
	member->mType = this;
	memset(member->mData, 0, sizeOfMemberData());
	insertLast(member);
	return member;
}

void CB_Type::deleteMember(CB_TypeMember *m) {
	assert(m->type() == this);
	if (m->after()) {
		m->after()->mBefore = m->before();
		if (m->before()) {
			m->before()->mAfter = m->after();
		}
		else { //m is the first member
			mFirst = m->after();
		}
	}
	else { //m is the last member
		if (!m->before()) {
			m->before()->mAfter = 0;
			mLast = m->before();

		}
		else { //m is the only member
			mFirst = 0;
			mLast = 0;
		}
	}

	delete [] reinterpret_cast<char*>(m);
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


void CB_TypeMember::deleteThis() {
	mType->deleteMember(this);
}
