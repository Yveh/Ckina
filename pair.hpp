#ifndef PAIR_HPP
#define PAIR_HPP
namespace sjtu {
	template<class T1, class T2>
	class pair {
	public:
		T1 first;
		T2 second;
		pair(const T1 &x, const T2 &y) : first(x), second(y) {}
		template<class U1, class U2>
		pair(U1 &&x, U2 &&y) : first(x), second(y) {}
		template<class U1, class U2>
		pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}
		template<class U1, class U2>
		pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {}
	};
}
#endif