#pragma once
#include "method.hpp"
#include "../utilities/array.hpp"

class InstanceKlass {
public:
	char pad_0000[280]; //0x0000
	void* _constants; //0x0118
	void* _inner_classes; //0x0120
	char pad_0128[48]; //0x0128
	void* _init_thread; //0x0158
	int32_t _vtable_len; //0x0160
	int32_t _itable_len; //0x0164
};