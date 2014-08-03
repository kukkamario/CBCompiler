#ifndef TYPES_H
#define TYPES_H
#include "common.h"

struct CB_Type;
/**
 * @brief The CB_TypeMember struct is a member of CB_Type.
 *
 * DO NOT CHANGE THE ORDER OF THE MEMBERS OF CB_TypeMember
 */
struct CB_TypeMember {
		friend struct CB_Type;
		CB_TypeMember *deleteThis();

		CB_TypeMember *after() const { return mAfter; }
		CB_TypeMember *before() const { return mBefore; }
		CB_Type *type() const { return mType; }
		void *data() { return static_cast<void*>(mData); }
	private:
		// 1. mAfter, 2. mBefore, 3. mType, 4. mData
		CB_TypeMember *mAfter;
		CB_TypeMember *mBefore;
		CB_Type *mType;

		/**
		 * @brief mData is pointer to the start of the data
		 */
		char mData[1];
};

/**
 * @brief The CB_Type struct is the base of the linked list.
 */
struct CB_Type {
		//friend void CBF_CB_ConstructType(CB_Type *, unsigned int);
	public:
		void construct(unsigned int sizeOfMember);
		CB_TypeMember *createMemberToEnd();
		CB_TypeMember *firstMember() { return mFirst; }
		CB_TypeMember *lastMember() { return mLast; }
		CB_TypeMember *deleteMember(CB_TypeMember *m);
		void setFirstMember(CB_TypeMember *m);
		void setLastMember(CB_TypeMember *m);
		void setSizeOfMember(unsigned int s) { mSizeOfMember = s; }
		unsigned int sizeOfMember() { return mSizeOfMember; }
		unsigned int sizeOfMemberData() { return mSizeOfMember - offsetof(CB_TypeMember, mData); }
	private:
		void insertLast(CB_TypeMember *member);
		CB_TypeMember *mFirst;
		CB_TypeMember *mLast;
		CB_TypeMember mDummyFirst;
		unsigned int mSizeOfMember;
};

typedef CB_Type Type;
typedef CB_TypeMember TypeMember;

#endif // TYPES_H
