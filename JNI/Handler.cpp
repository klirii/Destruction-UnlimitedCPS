#include "Handler.h"

HMODULE JNIHandler::jvm = GetModuleHandleA("jvm.dll");
JavaVM* JNIHandler::vm	= nullptr;

JNIEnv* JNIHandler::env = nullptr;
jobject JNIHandler::ClassLoader = nullptr;

JNIHandler::pJVM_FindClassFromCaller JNIHandler::JVM_FindClassFromCaller = reinterpret_cast<JNIHandler::pJVM_FindClassFromCaller>(GetProcAddress(JNIHandler::jvm, "IIllIlIIIlll"));
JNIHandler::pJVM_FindLoadedClass JNIHandler::JVM_FindLoadedClass = reinterpret_cast<JNIHandler::pJVM_FindLoadedClass>(GetProcAddress(JNIHandler::jvm, "IllIIlIllIlI"));

void JNIHandler::setVM() {
	typedef jint(JNICALL* pJNI_GetCreatedJavaVMs)(JavaVM** vmBuf, jsize bufLen, jsize* nVMs);
	pJNI_GetCreatedJavaVMs JNI_GetCreatedJavaVMs = reinterpret_cast<pJNI_GetCreatedJavaVMs>(GetProcAddress(JNIHandler::jvm, "IIIIlllllIIl"));
	JNI_GetCreatedJavaVMs(&JNIHandler::vm, 1, nullptr);
}

void JNIHandler::setEnv() {
	JNIHandler::vm->AttachCurrentThread(reinterpret_cast<void**>(&JNIHandler::env), nullptr);
}

void JNIHandler::setClassLoader() {
	jclass clazz = JNIHandler::env->FindClass("net/xtrafrancyz/vl/iIii");
	jfieldID fid = JNIHandler::env->GetStaticFieldID(clazz, " ", "Lnet/xtrafrancyz/vl/IiiIiI;");
	JNIHandler::ClassLoader = JNIHandler::env->GetStaticObjectField(clazz, fid);
}

jclass JNIHandler::FindClassFromCaller(const char* name) {
	return JVM_FindClassFromCaller(JNIHandler::env, name, true, JNIHandler::ClassLoader, nullptr);
}

jclass JNIHandler::FindLoadedClass(const char* name) {
	return JVM_FindLoadedClass(JNIHandler::env, JNIHandler::ClassLoader, env->NewStringUTF(name));
}