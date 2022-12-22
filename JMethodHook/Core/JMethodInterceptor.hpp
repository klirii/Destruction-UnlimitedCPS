#pragma once
#include "RegistersInterceptor.hpp"
#include "../JVM/hotspot/src/share/vm/oops/method.hpp"

#include <functional>
#include <jni.h>

namespace RegistersInterceptor {
	class JMethodInterceptor : public Interceptor {
	public:
		uint64_t callCounter = 0;

		JMethodInterceptor(jmethodID mid) {
			this->address = reinterpret_cast<PVOID>(*(reinterpret_cast<uintptr_t*>(mid)));
		}
		JMethodInterceptor(PVOID address) {
			this->address = address;
		}

		JMethodInterceptor(jmethodID mid, std::function<USHORT(uintptr_t)> asmInsert) {
			this->address = reinterpret_cast<PVOID>(*(reinterpret_cast<uintptr_t*>(mid)));
			this->asmInsert = asmInsert;
		}
		JMethodInterceptor(PVOID address, std::function<USHORT(uintptr_t)> asmInsert) {
			this->address = address;
			this->asmInsert = asmInsert;
		}

		void intercept() {
			Method* method = reinterpret_cast<Method*>(this->address);
			PVOID alloc = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

			// Записываем регистры в this->registers
			*reinterpret_cast<byte*>(alloc) = 0x52; // push rdx для сохранения значения регистра
			this->WriteRegistersInStruct(reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(alloc) + 1));

			// Увеличиваем this->callCounter на 1
			byte inc[6] = { 0xFF, 0x82, 0xD0, 0x04, 0x00, 0x00 };
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(alloc) + 120), inc, 6, nullptr);

			USHORT offset2jmp = 126;
			if (this->asmInsert) offset2jmp += this->asmInsert(reinterpret_cast<uintptr_t>(alloc) + offset2jmp);

			*reinterpret_cast<byte*>(reinterpret_cast<uintptr_t>(alloc) + offset2jmp) = 0x5A; // pop rdx для сохранения значения регистра

			// Делаем jmp на method->_code ? method->_from_interpreted_entry : method->_i2i_entry
			AssemblerAPI::Assembler::jmp(reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(alloc) + (offset2jmp + 1)),
				method->_code ? method->_from_interpreted_entry : method->_i2i_entry);

			// Записываем в структуру метода новый _from_interpreted_entry, указывающий на наш alloc
			byte newEntry[8];
			ReadProcessMemory(GetCurrentProcess(), &alloc, newEntry, 8, nullptr);
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(method) + 0x50), newEntry, 8, nullptr);

			// Перезаписываем хук в структуру метода, в случае его перекомпиляции
			while (true) {
				if (*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(method) + 0x50) != reinterpret_cast<uintptr_t>(alloc)) {
					AssemblerAPI::Assembler::jmp(reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(alloc) + (offset2jmp + 1)),
						method->_code ? method->_from_interpreted_entry : method->_i2i_entry);
					WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(method) + 0x50), newEntry, 8, nullptr);
				}
				Sleep(1000);
			}
		}
	private:
		PVOID address;
		std::function<USHORT(uintptr_t)> asmInsert;
	};
}