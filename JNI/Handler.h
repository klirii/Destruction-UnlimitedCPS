#pragma once
#include <jni.h>
#include <Windows.h>

class JNIHandler {
public:
	static HMODULE jvm;
	static JNIEnv* env;
	static jobject ClassLoader;

	static void setVM();
	static void setEnv();
	static void setClassLoader();

	static jclass FindClass(const char* name);
	static jclass FindAnyClass(const char* name);
private:
	static JavaVM* vm;

	typedef jclass(JNICALL* pJVM_FindClassFromCaller)(JNIEnv* env, const char* name, jboolean init, jobject loader, jclass caller);
	static  pJVM_FindClassFromCaller JVM_FindClassFromCaller;

	typedef jclass(JNICALL* pJVM_FindLoadedClass)(JNIEnv* env, jobject loader, jstring name);
	static  pJVM_FindLoadedClass JVM_FindLoadedClass;
};