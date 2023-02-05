#pragma once
#include <Windows.h>

#define METHOD_NOT_FOUND 0x01
#define CLASS_NOT_FOUND 0x02

#define KEYBOARD_HOOK_ERROR 0x03
#define MOUSE_HOOK_ERROR 0x04

namespace Utils {
	class ErrorHandler {
	public:
		static HWND window;

		static void send(int8_t error) {
			char errorMessage[10];
			sprintf_s(errorMessage, "Ошибка #%d", error);
			MessageBoxA(window, errorMessage, "Destruction", MB_ICONERROR);
		}
	};

	HWND ErrorHandler::window = FindWindowA(nullptr, "VimeWorld");
}