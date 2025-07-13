#include <jni.h>
#include <android/binder_ibinder_jni.h>

#include "EncryptServer.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_sdt_aidl_Native_getBinder(JNIEnv *env, jclass clazz) {
    LOGI("Java_com_sdt_aidl_Native_getBinder");
    std::shared_ptr<BnEncryptServer> server = ndk::SharedRefBase::make<BnEncryptServer>();
    jobject result = AIBinder_toJavaBinder(env, server->asBinder().get());
    LOGI("Java_com_sdt_aidl_Native_getBinder : %p" , result );

    return result;
}

