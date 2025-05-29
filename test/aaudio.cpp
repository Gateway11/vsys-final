// native-lib.cpp

#include <math.h>
#include <jni.h>
#include <aaudio/AAudio.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "AAudioDemo", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "AAudioDemo", __VA_ARGS__)

AAudioStream* audioStream = nullptr;

// 音频数据生成回调
aaudio_data_callback_result_t dataCallback(
    AAudioStream* stream,
    void* userData,
    void* audioData,
    int32_t numFrames) {
    
    // 生成测试音频（1kHz 正弦波）
    static float phase = 0.0f;
    const float phaseIncrement = 2.0f * M_PI * 1000.0f / AAudioStream_getSampleRate(stream);
    float* output = (float*)audioData;
    
    for (int i = 0; i < numFrames; ++i) {
        output[i] = sinf(phase);
        phase += phaseIncrement;
        if (phase > 2 * M_PI) phase -= 2 * M_PI;
    }
    
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_audiodemo_MainActivity_startPlayback(JNIEnv* env, jobject thiz) {
    AAudioStreamBuilder* builder;
    AAudio_createStreamBuilder(&builder);
    
    // 配置音频参数
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
    AAudioStreamBuilder_setChannelCount(builder, 2);
    AAudioStreamBuilder_setDataCallback(builder, dataCallback, nullptr);
    
    // 创建音频流
    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &audioStream);
    if (result != AAUDIO_OK) {
        LOGE("Stream open failed: %s", AAudio_convertResultToText(result));
        return;
    }
    
    // 启动音频流
    result = AAudioStream_requestStart(audioStream);
    if (result != AAUDIO_OK) {
        LOGE("Stream start failed: %s", AAudio_convertResultToText(result));
    }
    
    AAudioStreamBuilder_delete(builder);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_audiodemo_MainActivity_stopPlayback(JNIEnv* env, jobject thiz) {
    if (audioStream) {
        AAudioStream_requestStop(audioStream);
        AAudioStream_close(audioStream);
        audioStream = nullptr;
    }
}

int main() {
    Java_com_example_audiodemo_MainActivity_startPlayback(NULL, 0);
}
