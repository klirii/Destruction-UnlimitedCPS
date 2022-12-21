#pragma once
#include "constantPool.hpp"

class ConstMethod {
public:
	uint64_t _fingerprint;
	ConstantPool* _constants;

	void* _stackmap_data;

	int _constMethod_size;
	unsigned short _flags;
	uint8_t _result_type;

	unsigned short _code_size;
	unsigned short _name_index;
	unsigned short _signature_index;
};