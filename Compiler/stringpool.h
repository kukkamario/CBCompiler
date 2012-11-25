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
		llvm::Value *globalString(llvm::IRBuilder<> *builder, const QString &s);
	private:
		QMap<QString, llvm::Value*> mStrings;
};

#endif // STRINGPOOL_H
