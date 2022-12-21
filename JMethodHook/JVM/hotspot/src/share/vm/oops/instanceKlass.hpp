#pragma once
#include "symbol.hpp"

class InstanceKlass {
public:
	void* vTable;

	int _layout_helper;
	unsigned int _super_check_offset;

	Symbol* _name;

	char pad[192];
	void* _constants;
};