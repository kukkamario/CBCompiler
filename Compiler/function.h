#ifndef FUNCTION_H
#define FUNCTION_H
#include <vector>
#include <string>
#include "codepoint.h"
class ValueType;
class Builder;
class Value;
class FunctionValueType;

namespace llvm {
	class Function;
}

class Function {
	public:
		typedef std::vector<ValueType*> ParamList;
		Function(const std::string &name, const CodePoint &cp);
		virtual ~Function() { }
		std::string name() const{return mName;}

		/**
		 * @brief Function return ValueType getter. Will return 0 if Function returns void (command).
		 * @return ValueType of the return value of the function
		 */
		ValueType *returnValue() {return mReturnValue;}

		/**
		 * @brief requiredParams
		 * @return How many parameters are required, the others are optional and have default values.
		 */
		int requiredParams() const { return mRequiredParams;}

		/**
		 * @brief A list of the parameters
		 * @return std::vector<ValueType*> list of the parameters' ValueTypes.
		 */
		const ParamList &paramTypes() const {return mParamTypes;}
		llvm::Function *function()const{return mFunction;}

		boost::string_ref file() const { return mCodePoint.file(); }
		/**
		 * @return Line number where Function is defined or 0.
		 */
		int line() const { return mCodePoint.line(); }

		const CodePoint &codePoint() const { return mCodePoint; }

		/**
		 * @brief isRuntimeFunction
		 * @return True, if the function is defined in the runtime library. False otherwise.
		 */
		virtual bool isRuntimeFunction() const = 0;

		/**
		 * @brief isCommand
		 * @return True, if the function doesn't have return value and is a command. False otherwise.
		 */
		bool isCommand() const {return mReturnValue == 0;}

		/**
		 * @brief Casts parameters to right ValueTypes and creates a call instruction.
		 * @param builder Pointer to the builder
		 * @param params parameters
		 * @return Return value with ValueType of returnValue(). If the function is a command, isValid() == false.
		 */
		virtual Value call(Builder *builder, const std::vector<Value> &params) = 0;

		virtual FunctionValueType *functionValueType() const = 0;
	protected:
		std::string mName;
		ValueType *mReturnValue;
		ParamList mParamTypes;
		llvm::Function *mFunction;
		CodePoint mCodePoint;
		int mRequiredParams;
};

#endif // FUNCTION_H
