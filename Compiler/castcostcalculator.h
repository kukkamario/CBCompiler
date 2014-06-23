#ifndef CASTCOSTCALCULATOR_H
#define CASTCOSTCALCULATOR_H
#include "valuetype.h"
class CastCostCalculator {
	public:
		CastCostCalculator();
		CastCostCalculator(const CastCostCalculator &c);
		~CastCostCalculator();

		static CastCostCalculator maxCastCost();

		CastCostCalculator &operator=(const CastCostCalculator &o);
		CastCostCalculator &operator += (ValueType::CastCost cc);

		bool operator== (const CastCostCalculator &o) const;
		bool operator < (const CastCostCalculator &o) const;
		bool operator > (const CastCostCalculator &o) const;
		bool operator <= (const CastCostCalculator &o) const;
		bool operator >= (const CastCostCalculator &o) const;

		bool isCastPossible() const;
		void reset();
	private:
		int mCostArray[ValueType::ccNoCast];
		bool mNoCast;
};

#endif // CASTCOSTCALCULATOR_H
