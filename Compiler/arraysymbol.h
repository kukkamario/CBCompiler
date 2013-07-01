#ifndef ARRAYSYMBOL_H
#define ARRAYSYMBOL_H
#include "symbol.h"
#include "llvm.h"
class ValueType;
class Builder;

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

		/**
		 * @brief globalArrayData returns a pointer to the global variable which contains the data of the array.
		 */
		llvm::GlobalVariable *globalArrayData() const { return mGlobalArrayData; }
		/**
		 * @brief globalDimensionSizes returns a pointer to the global variable which contains a integer array representing sizes of the dimensions of the array.
		 * The integer array has dimensions() elements.
		 */
		llvm::GlobalVariable *globalDimensionSizes() const { return mGlobalDimensionSizes; }

		void createGlobalVariables(Builder *builder);
	private:
		ValueType *mValueType;
		int mDimensions;

		llvm::GlobalVariable *mGlobalArrayData;
		llvm::GlobalVariable *mGlobalDimensionSizes;
};

#endif // ARRAYSYMBOL_H
