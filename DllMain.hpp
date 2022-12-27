#pragma once
#include <Windows.h>
#include <thread>

#include "JNI/Handler.h"
#include "Utils/PatternScanner.hpp"
#include "Utils/ErrorHandler.hpp"

#include "JMethodHook/JVM/hotspot/src/share/vm/oops/instanceKlass.hpp"
#include "JMethodHook/Core/JMethodInterceptor.hpp"

#pragma warning(disable:4312)