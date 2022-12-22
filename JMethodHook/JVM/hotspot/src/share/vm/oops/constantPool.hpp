#pragma once
#include "instanceKlass.hpp"
#include <jni.h>

class ConstantPool {
public:
	void* vTable;

	void* _tags;
	void* _cache;
	InstanceKlass* _pool_holder;
	void* _operands;

	jobject _resolved_references;
	void* _reference_map;

	enum {
		_has_presolution = 1,
		_on_stack = 2
	};

	int _flags;
	int _length;

	union {
		int _resolved_reference_length;
		int _version;
	} _saved;

	jobject _lock;

	intptr_t* base() const { return (intptr_t*)(((char*)this) + sizeof(ConstantPool)); }

	Symbol** symbol_at_addr(int idx) const {
		return reinterpret_cast<Symbol**>(&base()[idx]);
	}
	Symbol* symbol_at(int idx) const {
		return *symbol_at_addr(idx);
	}
};