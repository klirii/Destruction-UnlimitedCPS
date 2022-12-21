#pragma once
#include "constMethod.hpp"

class Method {
public:
	void* vTable;

	ConstMethod* _constMethod;
	void* _method_data;
	void* _method_counters;

	uint8_t _access_flags[8];
	int _vtable_index;

	unsigned short _method_size;
	uint8_t _intrinsic_id;
	uint8_t _flags;

	void* _i2i_entry;
	void* _adapter;
	void* _from_compiled_entry;
	void* _code;
	void* _from_interpreted_entry;
};