#include "Handler.h"

#pragma warning(disable:6387)

HMODULE JNIHandler::jvm = nullptr;
JavaVM* JNIHandler::vm	= nullptr;

JNIEnv* JNIHandler::env = nullptr;
jobject JNIHandler::ClassLoader = nullptr;

JNIHandler::pJVM_FindClassFromCaller JNIHandler::JVM_FindClassFromCaller = nullptr;
JNIHandler::pJVM_FindLoadedClass JNIHandler::JVM_FindLoadedClass = nullptr;

void JNIHandler::initStaticFields() {
	jvm = GetModuleHandleA("jvm.dll");
	JVM_FindClassFromCaller = reinterpret_cast<JNIHandler::pJVM_FindClassFromCaller>(GetProcAddress(JNIHandler::jvm, "IIllIlIIIlll"));
	JVM_FindLoadedClass = reinterpret_cast<JNIHandler::pJVM_FindLoadedClass>(GetProcAddress(JNIHandler::jvm, "IllIIlIllIlI"));
}

void JNIHandler::setVM() {
	typedef jint(JNICALL* pJNI_GetCreatedJavaVMs)(JavaVM** vmBuf, jsize bufLen, jsize* nVMs);
	pJNI_GetCreatedJavaVMs JNI_GetCreatedJavaVMs = reinterpret_cast<pJNI_GetCreatedJavaVMs>(GetProcAddress(JNIHandler::jvm, "IIIIlllllIIl"));
	JNI_GetCreatedJavaVMs(&JNIHandler::vm, 1, nullptr);
}

void JNIHandler::setEnv() {
	JNIHandler::vm->AttachCurrentThread(reinterpret_cast<void**>(&JNIHandler::env), nullptr);
}

void JNIHandler::setClassLoader() {
	HMODULE vimeworld = GetModuleHandleA("VimeWorld.exe");

	if (vimeworld) {
		jclass MinecraftLoader = env->FindClass("net/xtrafrancyz/vl/iIIIi");
		if (!MinecraftLoader) return;

		jfieldID class_loader_fid = env->GetStaticFieldID(MinecraftLoader, " ", "Lnet/xtrafrancyz/vl/IIiI;");

		ClassLoader = env->GetStaticObjectField(MinecraftLoader, class_loader_fid);
		env->DeleteLocalRef(MinecraftLoader);
	}
	else {
		jclass Launcher = env->FindClass("sun/misc/Launcher");
		if (!Launcher) return;

		jmethodID get_launcher_mid = env->GetStaticMethodID(Launcher, "getLauncher", "()Lsun/misc/Launcher;");
		if (!get_launcher_mid) return;

		jmethodID get_class_loader_mid = env->GetMethodID(Launcher, "getClassLoader", "()Ljava/lang/ClassLoader;");
		if (!get_class_loader_mid) return;

		jobject launcher = env->CallStaticObjectMethod(Launcher, get_launcher_mid);
		if (launcher) {
			ClassLoader = env->CallObjectMethod(launcher, get_class_loader_mid);
			env->DeleteLocalRef(launcher);
		}

		env->DeleteLocalRef(Launcher);
	}
}

jclass JNIHandler::FindClassFromCaller(const char* name) {
	return JVM_FindClassFromCaller(JNIHandler::env, name, true, JNIHandler::ClassLoader, nullptr);
}

jclass JNIHandler::FindLoadedClass(const char* name) {
	return JVM_FindLoadedClass(JNIHandler::env, JNIHandler::ClassLoader, env->NewStringUTF(name));
}

jclass JNIHandler::FindClass(const char* name, jobject loader) {
	jclass ClassLoader = JNIHandler::env->FindClass("java/lang/ClassLoader");
	if (ClassLoader) {
		jmethodID load_class_mid = JNIHandler::env->GetMethodID(ClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
		JNIHandler::env->DeleteLocalRef(ClassLoader);

		jstring name_obj = JNIHandler::env->NewStringUTF(name);
		if (load_class_mid && name_obj) return (jclass)JNIHandler::env->CallObjectMethod(loader, load_class_mid, name_obj);
	}

	return nullptr;
}