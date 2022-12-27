#pragma once
template <typename T> class Array {
public:
	int _length;
	T	_data[1];

	T at(int idx) const {
		return _data[idx];
	}
};