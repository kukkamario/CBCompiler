#ifndef STRINGPOOL_H
#define STRINGPOOL_H
#include <QString>
#include <QLinkedList>
#include <QMap>
#include <llvm/IRBuilder.h>
namespace llvm {
	class Value;
}
class StringPool {
	public:
		StringPool();

		/**
		 * @brief globalString creates global variables for saving the string data.
		 *	Strings are cached and globalString will return same pointer if same string is requested multiple times.
		 * @param builder
		 *	IRBuilder for creating global variable if needed
		 * @param s
		 *	Global variable will be initialized with this string data
		 * @return Pointer to global constant string data (i8*)
		 */
		llvm::Value *globalString(llvm::IRBuilder<> *builder, const QString &s);
	private:
		QMap<QString, llvm::Value*> mStrings;
};

#endif // STRINGPOOL_H
