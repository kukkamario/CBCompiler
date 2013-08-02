#ifndef FUNCTION_H
#define FUNCTION_H
#include <QList>
#include <QString>
class ValueType;
class QFile;
class Builder;
class Value;
namespace llvm {
	class Function;
}
/**
 * @brief The Function is superclass of all functions callable in code.
 * @author Lassi Hämäläinen
 */
class Function {
	public:
		typedef QList<ValueType*> ParamList;
		Function(const QString &name, QFile *f, int line);
		virtual ~Function() { }
		QString name() const{return mName;}

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
		 * @return QList<ValueType*> list of the parameters' ValueTypes.
		 */
		const ParamList &paramTypes() const {return mParamTypes;}
		llvm::Function *function()const{return mFunction;}

		/**
		 * @return Pointer to file where Function is defined or null.
		 */
		QFile *file() const { return mFile; }
		/**
		 * @return Line number where Function is defined or 0.
		 */
		int line() const { return mLine; }

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
		virtual Value call(Builder *builder, const QList<Value> &params) = 0;
	protected:
		QString mName;
		ValueType *mReturnValue;
		ParamList mParamTypes;
		int mRequiredParams;
		llvm::Function *mFunction;
		QFile *mFile;
		int mLine;
};

#endif // FUNCTION_H
