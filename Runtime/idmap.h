#ifndef IDMAP_H
#define IDMAP_H
#include <map>
#include <unordered_map>
#include <stdexcept>

template <typename T>
class IdMap {
	public:
		IdMap();
		int add(T *val);
		T *get(int id) const;
		void remove(int id);
	private:
		std::unordered_map<int, T*> mMap;
		int mIdCounter;
};

template <typename T>
IdMap<T>::IdMap() :
	mIdCounter(0) {
}

template <typename T>
int IdMap<T>::add(T *val) {
	mMap[++mIdCounter] = val;
	return mIdCounter;
}

template <typename T>
T *IdMap<T>::get(int id) const {
	try {
		return mMap.at(id);
	}
	catch(std::out_of_range &e) {
		return 0;
	}
}

template <typename T>
void IdMap<T>::remove(int id) {
	assert(mMap.erase(id) == 1);
}


#endif // IDMAP_H
