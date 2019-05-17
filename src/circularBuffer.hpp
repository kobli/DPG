#ifndef CIRCULARBUFFER_HPP_19_05_17_16_22_29
#define CIRCULARBUFFER_HPP_19_05_17_16_22_29 
#include <vector>
#include <numeric>

template <typename T>
class CircularBuffer {
	public:
		CircularBuffer(unsigned size):
			_back{0}
		{
			_buff.resize(size);
		}

		void add(const T& v) {
			_buff[_back] = v;
			_back = (_back+1)%_buff.size();
		}

		T avg() {
			return std::accumulate(_buff.begin(), _buff.end(), T{})/_buff.size();
		}

	private:
		std::vector<T> _buff;
		unsigned _back;
};
#endif /* CIRCULARBUFFER_HPP_19_05_17_16_22_29 */
