#pragma once

#include <Windows.h>
#include <thread>
#include <jni.h>

class Method;
typedef unsigned __int8  u1;

typedef void(__cdecl* original_t)();

class Assembler {
public:
	// ������ ������ �� ���������� x64 �����
	static BYTE* jmp64(PVOID from, PVOID to) {
		BYTE jmp[14] = {0xFF, 0x25};
		ReadProcessMemory(GetCurrentProcess(), &to, jmp + 6, 8, nullptr);
		WriteProcessMemory(GetCurrentProcess(), from, jmp, 14, nullptr);
		return reinterpret_cast<BYTE*>(from) + 14;
	}
};

class JavaHook {
public:
	CONTEXT registers = {NULL};
	original_t original = nullptr;
private:
	CONTEXT* pRegisters = &registers;
	uint8_t argCount = NULL;

	// ��������� �������� ���� ��������� � WINAPI ��������� CONTEXT
	BYTE* SaveRegisters(BYTE* address) {
		/*
		* push rbx
		* mov rbx, context
		* mov [rbx+78], rax
		* mov [rbx+80], rcx
		* mov [rbx+88], rdx
		* pop rbx
		* mov rax, rbx
		* mov rbx, context
		* mov [rbx+90], rax
		* mov rax, [rbx+78]
		* mov [rbx+98], rsp
		* mov [rbx+A0], rbp
		* mov [rbx+A8], rsi
		* mov [rbx+B0], rdi
		* mov [rbx+B8], r8
		* mov [rbx+C0], r9
		* mov [rbx+C8], r10
		* mov [rbx+D0], r11
		* mov [rbx+D8], r12
		* mov [rbx+E0], r13
		* mov [rbx+E8], r14
		* mov [rbx+F0], r15
		* mov rbx, [rbx+90]
		*/
		BYTE movRegistersInStruct[145] = {
			0x53, 0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x43, 0x78, 0x48, 0x89, 0x8B, 0x80, 0x00, 0x00, 0x00,
			0x48, 0x89, 0x93, 0x88, 0x00, 0x00, 0x00, 0x5B, 0x48, 0x8B, 0xC3, 0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48,
			0x89, 0x83, 0x90, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x43, 0x78, 0x48, 0x89, 0xA3, 0x98, 0x00, 0x00, 0x00, 0x48, 0x89, 0xAB, 0xA0, 0x00,
			0x00, 0x00, 0x48, 0x89, 0xB3, 0xA8, 0x00, 0x00, 0x00, 0x48, 0x89, 0xBB, 0xB0, 0x00, 0x00, 0x00, 0x4C, 0x89, 0x83, 0xB8, 0x00, 0x00,
			0x00, 0x4C, 0x89, 0x8B, 0xC0, 0x00, 0x00, 0x00, 0x4C, 0x89, 0x93, 0xC8, 0x00, 0x00, 0x00, 0x4C, 0x89, 0x9B, 0xD0, 0x00, 0x00, 0x00,
			0x4C, 0x89, 0xA3, 0xD8, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xAB, 0xE0, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xB3, 0xE8, 0x00, 0x00, 0x00, 0x4C,
			0x89, 0xBB, 0xF0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x9B, 0x90, 0x00, 0x00, 0x00
		};

		// ���������� � ������ ����� �������� ��������� CONTEXT 
		ReadProcessMemory(GetCurrentProcess(), &pRegisters, movRegistersInStruct + 3, 8, nullptr);
		ReadProcessMemory(GetCurrentProcess(), &pRegisters, movRegistersInStruct + 35, 8, nullptr);

		// ���������� ��� � ������ � ���������� ����� ��������� ����������
		WriteProcessMemory(GetCurrentProcess(), address, movRegistersInStruct, 145, nullptr);
		return address + 145;
	}

	// �������� ����� ������ ��� ���� � ���������� � ����� � �������� rsp � rbp
	BYTE* CreateNewStack(BYTE* address) {
		/*
		* mov rsp, stack
		* mov rbp, rsp
		*/
		BYTE movRspRbp[13] = {
			0x48, 0xBC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xEC
		};

		// ���������� � ������ ����� ������ �����
		LPVOID stack = VirtualAlloc(NULL, 4194304, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		stack = reinterpret_cast<LPVOID>(reinterpret_cast<BYTE*>(stack) + 2097152);
		ReadProcessMemory(GetCurrentProcess(), &stack, movRspRbp + 2, 8, nullptr);

		// ���������� ��� � ������ � ���������� ����� ��������� ����������
		WriteProcessMemory(GetCurrentProcess(), address, movRspRbp, 13, nullptr);
		return address + 13;
	}

	// ��������������� �������� ���������, ��������� � �������� �������� �� WINAPI ��������� CONTEXT
	BYTE* RestoreRegisters(BYTE* address) {
		/*
		* mov rbx, context
		* mov rax, [rbx+78]
		* mov rcx, [rbx+80]
		* mov rdx, [rbx+88]
		* mov rsp, [rbx+98]
		* mov rbp, [rbx+A0]
		* mov rsi, [rbx+A8]
		* mov rdi, [rbx+B0]
		* mov r8, [rbx+B8]
		* mov r9, [rbx+C0]
		* mov r10, [rbx+C8]
		* mov r11, [rbx+D0]
		* mov r12, [rbx+D8]
		* mov r13, [rbx+E0]
		* mov r14, [rbx+E8]
		* mov r15, [rbx+F0]
		* mov rbx, [rbx+90]
		*/
		BYTE movStructInRegisters[119] = {
			0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x43, 0x78, 0x48, 0x8B, 0x8B, 0x80, 0x00, 0x00, 0x00, 0x48,
			0x8B, 0x93, 0x88, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xA3, 0x98, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xAB, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x8B,
			0xB3, 0xA8, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xBB, 0xB0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x83, 0xB8, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x8B,
			0xC0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x93, 0xC8, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0x9B, 0xD0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xA3, 0xD8,
			0x00, 0x00, 0x00, 0x4C, 0x8B, 0xAB, 0xE0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xB3, 0xE8, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xBB, 0xF0, 0x00,
			0x00, 0x00, 0x48, 0x8B, 0x9B, 0x90, 0x00, 0x00, 0x00
		};

		// ���������� � ������ ����� �������� ��������� CONTEXT
		ReadProcessMemory(GetCurrentProcess(), &pRegisters, movStructInRegisters + 2, 8, nullptr);

		// ���������� ��� � ������ � ���������� ����� ��������� ����������
		WriteProcessMemory(GetCurrentProcess(), address, movStructInRegisters, 119, nullptr);
		return address + 119;
	}

	void Create(PVOID* _from_interpreted_entry_p, PVOID interceptor) {
		// ��������� ������ ��� ���
		BYTE* space = reinterpret_cast<BYTE*>(VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE));

		// ��������� �������� ���������, ������ ����� ���� � ������� �� ���-�������
		BYTE* nextInstruction = SaveRegisters(space);
		BYTE* jmpToInterceptor = CreateNewStack(nextInstruction);
		nextInstruction = Assembler::jmp64(jmpToInterceptor, interceptor);

		// ��������������� �������� ��������� � ������� �� ������������ _from_interpreted_entry
		nextInstruction = RestoreRegisters(nextInstruction);
		Assembler::jmp64(nextInstruction, *_from_interpreted_entry_p);

		// ������������� ����� ��������� ������ �� ���-������� ��� ��������� � �������� _from_interpreted_entry � ��������� ������ �� ���
		original = reinterpret_cast<original_t>(jmpToInterceptor + 14);
		*_from_interpreted_entry_p = space;

		while (true) {
			if (*_from_interpreted_entry_p != space) {
				Assembler::jmp64(nextInstruction, *_from_interpreted_entry_p);
				*_from_interpreted_entry_p = space;
			}
			Sleep(1000);
		}
	}
public:
	JavaHook(PVOID* _from_interpreted_entry_p, PVOID interceptor, uint8_t argCount) {
		std::thread interception(&JavaHook::Create, this, _from_interpreted_entry_p, interceptor);
		interception.detach();
		this->argCount = argCount;
	}
};