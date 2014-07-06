#ifndef GLOBAL_H
#define GLOBAL_H
#include <QString>


//For building with Qt4
#ifndef QStringLiteral
	#define QStringLiteral QString
#endif
class Runtime;
#endif // GLOBAL_H
