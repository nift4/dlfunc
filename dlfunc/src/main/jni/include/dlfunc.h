#ifndef YAHFA_DLFUNC_H
#define YAHFA_DLFUNC_H

#include <jni.h>
#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

// setup the env, must be called before the other functions
int dlfunc_init(JNIEnv *env);

void *dlfunc_do(JNIEnv *env, uintptr_t func, uintptr_t handle, uintptr_t symbol);

inline void *dlfunc_dlopen(JNIEnv *env, const char *filename, int flags) {
	return dlfunc_do(env, (uintptr_t)dlopen, (uintptr_t)filename, (uintptr_t)flags);
}

inline void *dlfunc_dlsym(JNIEnv *env, void *handle, const char *symbol) {
	return dlfunc_do(env, (uintptr_t)dlsym, (uintptr_t)handle, (uintptr_t)symbol);
}

#ifdef __cplusplus
}
#endif

#endif //YAHFA_DLFUNC_H
