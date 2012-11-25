#ifndef UNIONWRAPPER_H
#define UNIONWRAPPER_H
template <typename T>
class UnionWrapper {
	public:
		void construct();
		template <typename P1>
		void construct(const P1 &p1);
		template <typename P1, typename P2>
		void construct(const P1 &p1, const P2 &p2);
		template <typename P1, typename P2, typename P3>
		void construct(const P1 &p1, const P2 &p2, const P3 &p3);

		void destruct();

		T &operator *();
		const T &operator *() const;
		T *operator ->();
		const T *operator ->() const;
	private:
		char mData[sizeof T];
};

template<typename T>
void UnionWrapper<T>::construct() {
	new(mData) T();
}

template<typename T> template<typename P1>
void UnionWrapper<T>::construct(const P1 &p1) {
	new(mData) T(p1);
}

template<typename T> template<typename P1, typename P2>
void UnionWrapper<T>::construct(const P1 &p1, const P2 &p2) {
	new(mData) T(p1, p2);
}

template<typename T> template<typename P1, typename P2, typename P3>
void UnionWrapper<T>::construct(const P1 &p1, const P2 &p2, const P3 &p3) {
	new(mData) T(p1, p2, p3);
}

template<typename T>
void UnionWrapper<T>::destruct() {
	(*this)->~T();
}

template<typename T>
T &UnionWrapper<T>::operator *() {
	return *reinterpret_cast<T*>(mData);
}

template<typename T>
const T &UnionWrapper<T>::operator *() const{
	return *reinterpret_cast<const T*>(mData);
}

template<typename T>
T *UnionWrapper<T>::operator ->() {
	return reinterpret_cast<T*>(mData);
}

template<typename T>
const T *UnionWrapper<T>::operator ->() const{
	return reinterpret_cast<const T*>(mData);
}

#endif // UNIONWRAPPER_H
