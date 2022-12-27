#pragma once
#include "method.hpp"
#include "../utilities/array.hpp"

class InstanceKlass {
public:
	void* vTable;

	int _layout_helper;
	unsigned int _super_check_offset;
	Symbol* _name;

	char pad[184];
	void* _constants;
	Array<uint16_t>* _inner_classes;

	char* _source_debug_extension;
	Symbol* _array_name;

	int _nonstatic_field_size;
	int _static_field_size;
	uint16_t _generic_signature_index;
	uint16_t _source_file_name_index;
	uint16_t _static_oop_field_count;
	uint16_t _java_fields_count;
	int _nonstatic_oop_map_size;

	bool _is_marked_dependent;
	bool _has_unloaded_dependent;

	enum {
		_misc_rewritten = 1 << 0,
		_misc_has_nonstatic_fields = 1 << 1,
		_misc_should_verify_class = 1 << 2,
		_misc_is_anonymous = 1 << 3,
		_misc_is_contended = 1 << 4,
		_misc_has_default_methods = 1 << 5,
		_misc_declares_default_methods = 1 << 6,
		_misc_has_been_redefined = 1 << 7
	};

	uint16_t _misc_flags;
	uint16_t _minor_version;
	uint16_t _major_version;
	void* _init_thread;
	int _vtable_len;
	int _itable_len;
	void* volatile _oop_map_cache;
	void* _member_names;
	void* _jni_ids;
	jmethodID* _methods_jmethod_ids;
	void* _dependencies;
	void* _osr_nmethods_head;
	void* _breakpoints;
	InstanceKlass* _previous_versions;
	void* _cached_class_file;

	volatile uint16_t _idnum_allocated_count;

	uint8_t _init_state;
	uint8_t _reference_type;

	void* _jvmti_cached_class_field_map;

	Array<Method*>* _methods;
};