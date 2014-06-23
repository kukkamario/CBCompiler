#include "castcostcalculator.h"

CastCostCalculator::CastCostCalculator() :
	mCostArray{0},
	mNoCast(false)
{
}

CastCostCalculator::CastCostCalculator(const CastCostCalculator &o) :
	mNoCast(o.mNoCast) {
	std::copy(o.mCostArray, o.mCostArray + ValueType::ccNoCast, this->mCostArray);
}

CastCostCalculator::~CastCostCalculator() {

}

CastCostCalculator CastCostCalculator::maxCastCost() {
	CastCostCalculator cc;
	std::fill_n(cc.mCostArray, ValueType::ccNoCast, 100000);
	cc.mNoCast = true;
	return cc;
}

CastCostCalculator &CastCostCalculator::operator +=(ValueType::CastCost cc) {
	if (cc == ValueType::ccNoCast) {
		mNoCast = true;
		return *this;
	}
	++mCostArray[cc];
	return *this;
}

bool CastCostCalculator::isCastPossible() const {
	return !mNoCast;
}

CastCostCalculator &CastCostCalculator::operator=(const CastCostCalculator &o) {
	std::copy(o.mCostArray, o.mCostArray + ValueType::ccNoCast, this->mCostArray);
	this->mNoCast = o.mNoCast;
	return *this;
}

bool CastCostCalculator::operator==(const CastCostCalculator &o) const {
	return mNoCast == o.mNoCast && std::equal(this->mCostArray + 1, this->mCostArray + ValueType::ccNoCast, o.mCostArray + 1);
}

bool CastCostCalculator::operator <(const CastCostCalculator &o) const {
	if (this->mNoCast && !o.mNoCast) return false;
	if (!this->mNoCast && o.mNoCast) return true;

	for (int i = ValueType::ccNoCast - 1; i > 1; i++) {
		if (this->mCostArray[i] < o.mCostArray[i]) return true;
		if (this->mCostArray[i] > o.mCostArray[i]) return false;
	}
	return false;
}

bool CastCostCalculator::operator >(const CastCostCalculator &o) const {
	if (!this->mNoCast && o.mNoCast) return false;
	if (this->mNoCast && !o.mNoCast) return true;

	for (int i = ValueType::ccNoCast - 1; i > 1; i++) {
		if (this->mCostArray[i] > o.mCostArray[i]) return true;
		if (this->mCostArray[i] < o.mCostArray[i]) return false;

	}
	return false;
}

bool CastCostCalculator::operator <=(const CastCostCalculator &o) const {
	if (this->mNoCast && !o.mNoCast) return false;
	if (!this->mNoCast && o.mNoCast) return true;

	for (int i = ValueType::ccNoCast - 1; i > 1; i++) {
		if (this->mCostArray[i] < o.mCostArray[i]) return true;
		if (this->mCostArray[i] > o.mCostArray[i]) return false;
	}
	return true;
}

bool CastCostCalculator::operator >=(const CastCostCalculator &o) const {
	if (!this->mNoCast && o.mNoCast) return false;
	if (this->mNoCast && !o.mNoCast) return true;

	for (int i = ValueType::ccNoCast - 1; i > 1; i++) {
		if (this->mCostArray[i] > o.mCostArray[i]) return true;
		if (this->mCostArray[i] < o.mCostArray[i]) return false;

	}
	return true;
}

void CastCostCalculator::reset() {
	std::fill_n(mCostArray, ValueType::ccNoCast, 0);
	mNoCast = false;
}
