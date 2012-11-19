#ifndef STRINGPOOL_H
#define STRINGPOOL_H
#include <QString>
#include <QLinkedList>
#include <QMap>
namespace llvm {
	class Value;

	namespace ____dumb__ { class DumbClass;}
	template<bool preserveNames = true, typename T = ____dumb__::DumbClass,typename Inserter = ____dumb__::DumpClass>
	class IRBuilder;
}
class StringPool {
	public:
		StringPool();
		QString *add(const QString &s);
		llvm::Value *globalString(llvm::IRBuilder<> *builder, QString *s);
	private:
		struct StringItem {
				StringItem(const QString &s) : mString(new QString(s)), mGlobalString(0) {}
				QString *mString;
				llvm::Value *mGlobalString;
		};

		QMap<QString, StringItem> mStrings;
		QMap<QString*, QMap<QString, StringItem>::Iterator > mReverse;
};

#endif // STRINGPOOL_H
