#pragma once
#include <iostream>

class Symbol {
public:
	unsigned short _length;
	short _refcount;

	int _identity_hash;
	char _body[1];

	std::string as_string() {
		if (!_body) return "";
		return std::string(_body, _length);
	}
};