#ifndef YAHFA_DLFUNC_H
#define YAHFA_DLFUNC_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

// setup the env, must be called before the other functions
int dlfunc_init(JNIEnv *env);

// call dlopen(filename, flags)
void *dlfunc_dlopen(JNIEnv *env, const char *filename, int flags);

// call dlsym(handle, symbol)
void *dlfunc_dlsym(JNIEnv *env, void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

#endif //YAHFA_DLFUNC_H
