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
		friend void CBF_CB_ConstructType(CB_Type *, unsigned int);
	public:
		CB_TypeMember *createMemberToEnd();
		CB_TypeMember *firstMember() { return mFirst; }
		CB_TypeMember *lastMember() { return mLast; }
		unsigned int sizeOfMember() { return mSizeOfMember; }
		unsigned int sizeOfMemberData() { return mSizeOfMember - offsetof(CB_TypeMember, mData); }
	private:
		void insertLast(CB_TypeMember *member);
		CB_TypeMember *mFirst;
		CB_TypeMember *mLast;
		unsigned int mSizeOfMember;
		int mId;
};

typedef CB_Type Type;
typedef CB_TypeMember TypeMember;

#endif // TYPES_H
