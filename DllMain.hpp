#pragma once
#include <Windows.h>
#include <thread>

#include "JNI/Handler.h"
#include "Utils/PatternScanner.hpp"
#include "Utils/ErrorHandler.hpp"

#include "JMethodHook/JVM/hotspot/src/share/vm/oops/method.hpp"
#include "JMethodHook/Core/JMethodInterceptor.hpp"