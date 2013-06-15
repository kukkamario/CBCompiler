#ifndef ARRAYSYMBOL_H
#define ARRAYSYMBOL_H
#include "symbol.h"
class ValueType;
/**
 * @brief The ArraySymbol class.
 */
class ArraySymbol : public Symbol {
	public:
		ArraySymbol(const QString &name, ValueType *valType, int dim, QFile *file, int line);

		/**
		 * @brief Symbol type
		 * @return Symbol type
		 */
		Type type() const{return stArray;}

		/**
		 * @brief Debugging information in the string format. Only for the debug output.
		 * @return Debugging information
		 */
		QString info() const;

		/**
		 * @brief dimensions
		 * @return Amount of the dimensions the array has.
		 */
		int dimensions() const {return mDimensions;}

		/**
		 * @brief
		 * @return ValueType of the items of the array.
		 */
		ValueType *valueType()const {return mValueType;}
	private:
		ValueType *mValueType;
		int mDimensions;
};

#endif // ARRAYSYMBOL_H
