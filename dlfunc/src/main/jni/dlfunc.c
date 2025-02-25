#include "include/dlfunc.h"
#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "dlfunc"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#if defined(__arm__) || defined(__i386__)
#define METHOD_SIG "(III)I"
#define CLASS_NAME "dlfunc32"
#define is32Bit
#include "JNIHelper_dex32.h"
#elif defined(__aarch64__) || defined(__x86_64__)
#define METHOD_SIG "(JJJ)J"
#include "JNIHelper_dex64.h"
#define CLASS_NAME "dlfunc64"
#endif
#define METHOD_NAME "c"

static jclass jniHelper = NULL;
static jmethodID jniCall = NULL;

static void setupParam();

#if defined(__aarch64__)
__asm__(
        "setupParam:\n"
        "mov x0, x3\n"
        "mov x1, x4\n"
        "br x2\n"
        );
#elif defined(__x86_64__)
__asm__(
        "setupParam:\n"
        "movq %rcx, %rdi\n"
        "movq %r8, %rsi\n"
        "jmp *%rdx\n"
        );
#elif defined(__arm__)
__asm__(
        "setupParam:\n"
        ".arm\n"
        "mov r0, r3\n"
        "ldr r1, [sp]\n"
        "bx r2\n"
        );
#elif defined(__i386__)
__asm__(
        "setupParam:\n"
        "mov 16(%esp), %eax\n"
        "mov %eax, 4(%esp)\n"
        "mov 20(%esp), %eax\n"
        "mov %eax, 8(%esp)\n"
        "mov 12(%esp), %eax\n"
        "jmp *%eax\n"
        );
#else
#error "Unsupported architecture"
#endif

void *dlfunc_dlopen(JNIEnv *env, const char *filename, int flags) {
    void *handle = NULL;
    if (!jniHelper || !jniCall) {
        LOGE("env not setup, call dlfunc_init");
        return handle;
    }

#if defined(is32Bit)
    handle = (void *) (*env)->CallStaticIntMethod(env, jniHelper, jniCall, (int) dlopen,
                                                  (int) filename, (int) flags);
#else
    handle = (void *) (*env)->CallStaticLongMethod(env, jniHelper, jniCall, (long)dlopen,
												   (long)filename, (long)flags);
#endif
    return handle;
}

void *dlfunc_dlsym(JNIEnv *env, void *handle, const char *symbol) {
    void *ptr = NULL;
    if(!jniHelper || !jniCall) {
        LOGE("env not setup, call dlfunc_init");
        return ptr;
    }

#if defined(is32Bit)
    ptr = (void *) (*env)->CallStaticIntMethod(env, jniHelper, jniCall, (int) dlsym, (int) handle,
                                               (int) symbol);
#else
    ptr = (void *) (*env)->CallStaticLongMethod(env, jniHelper, jniCall, (long)dlsym, (long)handle,
												(long)symbol);
#endif
    return ptr;
}

// https://github.com/PAGalaxyLab/YAHFA/issues/161
// adb shell cmd package compile -m speed -f <package name>
// method could be aot compiled and the entrypoint would be replaced
// so we load the dex at runtime with InMemoryDexClassLoader
static jclass findHelperClass(JNIEnv* env) {
	if (android_get_device_api_level() < 29)
		return (*env)->FindClass(env, CLASS_NAME);
    jclass classLoader_class = (*env)->FindClass(env, "dalvik/system/InMemoryDexClassLoader");
    if(classLoader_class == NULL) {
        LOGE("cannot find InMemoryDexClassLoader");
        return NULL;
    }

    jmethodID classCtor = (*env)->GetMethodID(env, classLoader_class, "<init>",
                                              "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    if(classCtor == NULL) {
        LOGE("cannot find InMemoryDexClassLoader.<init>");
        return NULL;
    }

    // load the precompiled dex file
    jobject dexBuffer = (*env)->NewDirectByteBuffer(env, classes_dex, classes_dex_len);

    jobject classLoader = (*env)->NewObject(env, classLoader_class, classCtor, dexBuffer, NULL);
    if(classLoader == NULL) {
        LOGE("cannot init InMemoryDexClassLoader");
        return NULL;
    }

    jmethodID loadClass = (*env)->GetMethodID(env, classLoader_class, "loadClass",
                                              "(Ljava/lang/String;)Ljava/lang/Class;");
    if(loadClass == NULL) {
        LOGE("cannot find InMemoryDexClassLoader.loadClass");
        return NULL;
    }
    jclass targetClass = (*env)->CallObjectMethod(env, classLoader, loadClass,
                                                  (*env)->NewStringUTF(env, CLASS_NAME));

    return targetClass;
}

int dlfunc_init(JNIEnv* env) {
    if(!jniHelper || !jniCall) {
        jclass localClass = findHelperClass(env);
        if (localClass == NULL) {
            LOGE("cannot find class %s", CLASS_NAME);
            (*env)->ExceptionClear(env);
            return JNI_ERR;
        }

        static const JNINativeMethod methods[] = {
                {METHOD_NAME, METHOD_SIG, setupParam},
        };
        int rc = (*env)->RegisterNatives(env, localClass, methods, 1);
        if (rc != JNI_OK) {
            LOGE("failed to register native method %s%s at %p", METHOD_NAME, METHOD_SIG, setupParam);
            (*env)->ExceptionClear(env);
            return rc;
        }

        jniCall = (*env)->GetStaticMethodID(env, localClass, METHOD_NAME, METHOD_SIG);
        if (jniCall == NULL) {
            LOGE("failed to get static method %s%s", METHOD_NAME, METHOD_SIG);
            (*env)->ExceptionClear(env);
            return JNI_ERR;
        }
        jniHelper = (*env)->NewGlobalRef(env, localClass);
        LOGI("dlfunc_init done");
    }

    return JNI_OK;
}
