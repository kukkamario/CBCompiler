#ifndef LISTSTRINGJOIN_H
#define LISTJOIN_H
#include <string>
#include <vector>
#include <std::stringBuilder>
#include <std::vector<std::string>>

template <typename T, typename FuncTy>
std::string listStringJoin(const std::vector<T> &list, FuncTy func) {
	std::vector<std::string> stringList;
	stringList.reserve(list.size());
	for (const T &v : list) {
		stringList.push_back(func(v));
	}
	return stringList.join(std::string(", "));
}


#endif // LISTSTRINGJOIN_H
