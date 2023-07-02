#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <assert.h>
#include "vsys_activation.h"

#define TAG "Rokid.VoiceActivation.jni"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define CLASS_NAME_ONE "com/rokid/openvoice/VoiceActivationImpl"
#define CLASS_NAME_TWO "com/rokid/openvoice/VoiceActivationBuilder$CreateArgs"

static JavaVM* java_vm_instance_ = NULL;

typedef struct {
	VsysActivationInst handle;
	JNIEnv* env;
	jobject javaInstance;
	jmethodID javaCallback;
	vt_word_t* vt_words;
	int32_t vt_words_count;
} NativeVoiceActivation;

static void voice_activation_event_callback(voice_event_t* event, void* token) {
	NativeVoiceActivation* nva = (NativeVoiceActivation*)token;
	if (nva->env == NULL) {
		if (java_vm_instance_->AttachCurrentThreadAsDaemon(&nva->env, NULL) != JNI_OK) {
			LOGE("voice_activation_event_callback: attach current thread failed, event cannot callback to java function");
			return;
		}
	}

	jbyteArray data = NULL;
	nva->env->PushLocalFrame(16);
	if (event->length > 0) {
		data = nva->env->NewByteArray(event->length);
		jbyte* elems = nva->env->GetByteArrayElements(data, NULL);
		memcpy(elems, event->data, event->length);
		nva->env->ReleaseByteArrayElements(data, elems, 0);
	}
	nva->env->CallVoidMethod(nva->javaInstance, nva->javaCallback,
			event->event, event->vt_info.begin, event->vt_info.end,
			event->vt_info.energy, event->energy, event->threshold_energy,
			event->sound_location, data);
	nva->env->PopLocalFrame(NULL);
}

static jlong native_create_handle(JNIEnv* env, jobject thiz, jlong param, jstring basePath, jboolean vadEnabled, jobject inst) {
	activation_param_t* apt = reinterpret_cast<activation_param_t*>(param);
	VsysActivationInst handle;
	const char* path = env->GetStringUTFChars(basePath, NULL);
	handle = VsysActivation_Create(apt, path, vadEnabled);
	env->ReleaseStringUTFChars(basePath, path);

	if (handle == 0) {
		LOGE("native_create_handle: create failed");
		return 0;
	}
	NativeVoiceActivation* nva = new NativeVoiceActivation();
	memset(nva, 0, sizeof(NativeVoiceActivation));
	nva->handle = handle;
	nva->javaInstance = env->NewGlobalRef(inst);
	nva->javaCallback = env->GetMethodID(env->GetObjectClass(inst), "nativeEventCallback", "(IIIFFFF[B)V");
	if (nva->javaCallback == NULL) {
		LOGE("cannot find method 'nativeEventCallback' in java instance");
		delete nva;
		return 0;
	}
	VsysActivation_RegisterVoiceEventCallback(handle, voice_activation_event_callback, nva);
	return (jlong)nva;
}

static void native_free_handle(JNIEnv* env, jobject thiz, jlong handle) {
	NativeVoiceActivation* nva = (NativeVoiceActivation*)handle;
	VsysActivation_Free(nva->handle);
	env->DeleteGlobalRef(nva->javaInstance);
	delete nva;
}

static jint native_process(JNIEnv* env, jobject thiz, jlong handle, jbyteArray input, jint offset, jint length) {
	NativeVoiceActivation* nva = (NativeVoiceActivation*)handle;
	jbyte* data = env->GetByteArrayElements(input, NULL);
	int32_t r = VsysActivation_Process(nva->handle, (uint8_t*)data + offset, length);
	env->ReleaseByteArrayElements(input, data, JNI_ABORT);
	return (jint)r;
}

static jint native_control(JNIEnv* env, jobject thiz, jlong handle, jint state) {
	NativeVoiceActivation* nva = (NativeVoiceActivation*)handle;
	return VsysActivation_Control(nva->handle, (active_action)state);
}

static jint native_add_vt_word(JNIEnv* env, jobject thiz, jlong handle, jint type, jstring jword, jstring jphone) {
	NativeVoiceActivation* nva = (NativeVoiceActivation*)handle;
	vt_word_t vtword;
	memset(&vtword, 0, sizeof(vt_word_t));
	vtword.type = (word_type)type;
	const char* phone = env->GetStringUTFChars(jphone, NULL);
	const char* word = env->GetStringUTFChars(jword, NULL);
	strcpy(vtword.phone, phone);
	strcpy(vtword.word_utf8, word);
	int32_t r = VsysActivation_AddVtWord(nva->handle, &vtword);
	env->ReleaseStringUTFChars(jphone, phone);
	env->ReleaseStringUTFChars(jword, word);
	return (jint)r;
}

static jint native_remove_vt_word(JNIEnv* env, jobject thiz, jlong handle, jstring word) {
	NativeVoiceActivation* nva = (NativeVoiceActivation*)handle;
	const char* wstr = env->GetStringUTFChars(word, NULL);
	int32_t r = VsysActivation_RemoveVtWord(nva->handle, wstr);
	env->ReleaseStringUTFChars(word, wstr);
	return (jint)r;
}

static jint native_get_vt_words_count(JNIEnv* env, jobject thiz, jlong handle) {
	NativeVoiceActivation* nva = (NativeVoiceActivation*)handle;
	int32_t count;
	vt_word_t* words;
	count = VsysActivation_GetVtWords(nva->handle, &words);
	if (count < 0)
		return -1;
	nva->vt_words = words;
	nva->vt_words_count = count;
	return (jint)count;
}

static void native_fill_vt_words(JNIEnv* env, jobject thiz, jlong handle, jintArray types, jobjectArray words, jobjectArray phones) {
	NativeVoiceActivation* nva = (NativeVoiceActivation*)handle;
	if (nva->vt_words_count > 0) {
		jint* types_data = env->GetIntArrayElements(types, NULL);
		int32_t i;
		for (i = 0; i < nva->vt_words_count; ++i) {
			types_data[i] = nva->vt_words[i].type;
			env->SetObjectArrayElement(words, i, env->NewStringUTF(nva->vt_words[i].word_utf8));
			env->SetObjectArrayElement(phones, i, env->NewStringUTF(nva->vt_words[i].phone));
		}
		env->ReleaseIntArrayElements(types, types_data, 0);
	}
}

static jlong native_create_param(JNIEnv* env, jobject thiz, jint sampleRate, jint bits, jint channels, jint mask) {
	activation_param_t* apt = (activation_param_t*)malloc(sizeof(activation_param_t) + sizeof(mic_param_t) * channels);
	memset(apt, 0, sizeof(activation_param_t));
	apt->sample_rate = sampleRate;
	apt->sample_size_bits = bits;
	apt->num_channels = channels;
	apt->mask = mask;
	apt->mic_params = (mic_param_t*)(apt + 1);
	return (jlong)apt;
}

static void native_add_mic(JNIEnv* env, jobject thiz, jlong param, jint idx, jfloat x, jfloat y, jfloat z, jfloat delay) {
	activation_param_t* apt = (activation_param_t*)param;
	assert(apt->num_mics < apt->num_channels);
	mic_param_t* cpt = apt->mic_params + apt->num_mics;
	cpt->id = idx;
	cpt->position.x = x;
	cpt->position.y = y;
	cpt->position.z = z;
	cpt->delay = delay;
	++apt->num_mics;
}

static JNINativeMethod nativeCreateArgsMethods[] = {
	{ "native_create_param", "(IIII)J", (void*)native_create_param },
	{ "native_add_mic", "(JIFFFF)V", (void*)native_add_mic },
};

static JNINativeMethod nativeVoiceActivationMethods[] = {
	{ "native_create_handle", "(JLjava/lang/String;ZLcom/rokid/openvoice/VoiceActivationImpl;)J", (void*)native_create_handle },
	{ "native_free_handle", "(J)V", (void*)native_free_handle },
	{ "native_process", "(J[BII)I", (void*)native_process },
	{ "native_control", "(JI)I", (void*)native_control },
	{ "native_add_vt_word", "(JILjava/lang/String;Ljava/lang/String;)I", (void*)native_add_vt_word },
	{ "native_remove_vt_word", "(JLjava/lang/String;)I", (void*)native_remove_vt_word },
	{ "native_get_vt_words_count", "(J)I", (void*)native_get_vt_words_count },
	{ "native_fill_vt_words", "(J[I[Ljava/lang/String;[Ljava/lang/String;)V", (void*)native_fill_vt_words },
};

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env;

	java_vm_instance_ = vm;

	if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("%s: JNI_OnLoad failed", "RokidVoiceActivation");
		return -1;
	}
	jclass cls = env->FindClass(CLASS_NAME_ONE);
	if (cls == NULL) {
		LOGE("cannot find class %s", CLASS_NAME_ONE);
		return -1;
	}
	if (env->RegisterNatives(cls, nativeVoiceActivationMethods, sizeof(nativeVoiceActivationMethods) / sizeof(JNINativeMethod)) != 0)
		return -1;
	cls = env->FindClass(CLASS_NAME_TWO);
	if (cls == NULL) {
		LOGE("cannot find class %s", CLASS_NAME_TWO);
		return -1;
	}
	if (env->RegisterNatives(cls, nativeCreateArgsMethods, sizeof(nativeCreateArgsMethods) / sizeof(JNINativeMethod)) != 0)
		return -1;
	return JNI_VERSION_1_4;
}
