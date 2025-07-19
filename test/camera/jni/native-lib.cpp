#include <jni.h>
#include <android/log.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraCaptureSession.h>
#include <media/NdkImageReader.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#define TAG "CameraNDK"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

struct CameraContext {
    ACameraManager* cameraManager = nullptr;
    ACameraDevice* cameraDevice = nullptr;
    ACaptureSessionOutputContainer* outputContainer = nullptr;
    ACaptureSessionOutput* imageReaderOutput = nullptr;
    ACaptureSessionOutput* previewOutput = nullptr;
    ACameraCaptureSession* captureSession = nullptr;
    AImageReader* imageReader = nullptr;
    ANativeWindow* previewWindow = nullptr;  // 用于显示的Surface
    ACaptureRequest* captureRequest = nullptr;
    ACameraOutputTarget* imageReaderTarget = nullptr;
    ACameraOutputTarget* previewTarget = nullptr;
};

CameraContext* ctx = new CameraContext();

static void onImageAvailable(void* context, AImageReader* reader) {
    AImage* image = nullptr;
    media_status_t status = AImageReader_acquireNextImage(reader, &image);
    if (status == AMEDIA_OK && image != nullptr) {
        int32_t width, height, format;
        AImage_getWidth(image, &width);
        AImage_getHeight(image, &height);
        AImage_getFormat(image, &format);
        LOGI("Image acquired: %dx%d, format: %d", width, height, format);
        
        // 这里处理图像数据（例如：转换、分析等）
        // 注意：处理完成后必须删除image
        AImage_delete(image);
    }
}

static ACameraDevice_StateCallbacks cameraDeviceCallbacks = {
    .context = nullptr,
    .onDisconnected = [](void* context, ACameraDevice* device) {
        LOGI("Camera disconnected");
        ACameraDevice_close(device);
    },
    .onError = [](void* context, ACameraDevice* device, int error) {
        LOGE("Camera error: %d", error);
        ACameraDevice_close(device);
    }
};

static void onSessionClosed(void* context, ACameraCaptureSession* session) {
    LOGI("Capture session closed");
}

static void onSessionReady(void* context, ACameraCaptureSession* session) {
    LOGI("Capture session ready");
}

static void onSessionActive(void* context, ACameraCaptureSession* session) {
    LOGI("Capture session active");
}

static ACameraCaptureSession_stateCallbacks captureSessionCallbacks = {
    .context = nullptr,
    .onClosed = onSessionClosed,
    .onReady = onSessionReady,
    .onActive = onSessionActive
};

// 启动相机预览
// previewSurface: 从Java Surface传递过来的ANativeWindow
void startCamera(ANativeWindow* previewSurface) {
    ctx->previewWindow = previewSurface;

    // 创建相机管理器
    ctx->cameraManager = ACameraManager_create();

    // 获取相机列表
    ACameraIdList* cameraIdList = nullptr;
    camera_status_t status = ACameraManager_getCameraIdList(ctx->cameraManager, &cameraIdList);
    if (status != ACAMERA_OK || cameraIdList->numCameras == 0) {
        LOGE("No cameras available");
        return;
    }

    // 使用后置摄像头
    const char* selectedCameraId = nullptr;
    for (int i = 0; i < cameraIdList->numCameras; i++) {
        const char* id = cameraIdList->cameraIds[i];
        ACameraMetadata* metadata;
        if (ACameraManager_getCameraCharacteristics(ctx->cameraManager, id, &metadata) == ACAMERA_OK) {
            ACameraMetadata_const_entry lensFacing;
            ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &lensFacing);
            if (lensFacing.data.u8[0] == ACAMERA_LENS_FACING_BACK) {
                selectedCameraId = id;
                break;
            }
            ACameraMetadata_free(metadata);
        }
    }

    if (!selectedCameraId) {
        LOGE("No back-facing camera found");
        ACameraManager_deleteCameraIdList(cameraIdList);
        return;
    }

    // 打开相机
    cameraDeviceCallbacks.context = ctx;
    status = ACameraManager_openCamera(ctx->cameraManager, selectedCameraId,
                                      &cameraDeviceCallbacks, &ctx->cameraDevice);
    ACameraManager_deleteCameraIdList(cameraIdList);
    if (status != ACAMERA_OK) {
        LOGE("Failed to open camera: %d", status);
        return;
    }

    // 创建ImageReader (YUV格式用于处理)
    int32_t width = 1280;
    int32_t height = 720;
    int32_t maxImages = 2;
    if (AImageReader_new(width, height,
                AIMAGE_FORMAT_YUV_420_888, maxImages, &ctx->imageReader) != AMEDIA_OK) {
        LOGE("Failed to create ImageReader");
        return;
    }

    // 设置ImageReader回调
    AImageReader_ImageListener readerListener = {
        .context = ctx,
        .onImageAvailable = onImageAvailable
    };
    AImageReader_setImageListener(ctx->imageReader, &readerListener);

    // 获取ImageReader的窗口
    ANativeWindow* readerWindow;
    AImageReader_getWindow(ctx->imageReader, &readerWindow);

    // 创建输出容器
    ACaptureSessionOutputContainer_create(&ctx->outputContainer);

    // 添加ImageReader输出
    ACaptureSessionOutput_create(readerWindow, &ctx->imageReaderOutput);
    ACaptureSessionOutputContainer_add(ctx->outputContainer, ctx->imageReaderOutput);

    // 添加预览输出 (直接显示到Surface)
    ACaptureSessionOutput_create(ctx->previewWindow, &ctx->previewOutput);
    ACaptureSessionOutputContainer_add(ctx->outputContainer, ctx->previewOutput);

    // 创建捕获会话
    captureSessionCallbacks.context = ctx;
    status = ACameraDevice_createCaptureSession(ctx->cameraDevice, ctx->outputContainer,
                                               &captureSessionCallbacks, &ctx->captureSession);
    if (status != ACAMERA_OK) {
        LOGE("Failed to create capture session: %d", status);
        return;
    }

    // 创建预览请求
    ACameraDevice_createCaptureRequest(ctx->cameraDevice, TEMPLATE_PREVIEW, &ctx->captureRequest);

    // 添加ImageReader目标
    ACameraOutputTarget_create(readerWindow, &ctx->imageReaderTarget);
    ACaptureRequest_addTarget(ctx->captureRequest, ctx->imageReaderTarget);

    // 添加预览目标
    ACameraOutputTarget_create(ctx->previewWindow, &ctx->previewTarget);
    ACaptureRequest_addTarget(ctx->captureRequest, ctx->previewTarget);

    // 设置连续捕获请求
    status = ACameraCaptureSession_setRepeatingRequest(ctx->captureSession, nullptr,
                                                      1, &ctx->captureRequest, nullptr);
    if (status != ACAMERA_OK) {
        LOGE("Failed to start preview: %d", status);
    } else {
        LOGI("Camera preview started");
    }
}

// 清理资源
void stopCamera(CameraContext* ctx) {
    if (ctx->captureSession) {
        ACameraCaptureSession_stopRepeating(ctx->captureSession);
        ACameraCaptureSession_close(ctx->captureSession);
    }
    if (ctx->captureRequest) {
        ACaptureRequest_removeTarget(ctx->captureRequest, ctx->imageReaderTarget);
        ACaptureRequest_removeTarget(ctx->captureRequest, ctx->previewTarget);
        ACaptureRequest_free(ctx->captureRequest);
    }
    if (ctx->imageReaderTarget) ACameraOutputTarget_free(ctx->imageReaderTarget);
    if (ctx->previewTarget) ACameraOutputTarget_free(ctx->previewTarget);
    if (ctx->imageReaderOutput) ACaptureSessionOutput_free(ctx->imageReaderOutput);
    if (ctx->previewOutput) ACaptureSessionOutput_free(ctx->previewOutput);
    if (ctx->outputContainer) ACaptureSessionOutputContainer_free(ctx->outputContainer);
    if (ctx->imageReader) AImageReader_delete(ctx->imageReader);
    if (ctx->cameraDevice) ACameraDevice_close(ctx->cameraDevice);
    if (ctx->cameraManager) ACameraManager_delete(ctx->cameraManager);
    delete ctx;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_CameraActivity_startCameraNative(JNIEnv *env, jobject thiz, jobject surface) {
    ANativeWindow* previewWindow = ANativeWindow_fromSurface(env, surface);
    if (!previewWindow) {
        LOGE("Failed to get native window from surface");
        return 0;
    }

    // 调用之前的startCamera函数
    startCamera(previewWindow);

    // 这里需要返回CameraContext指针
    return reinterpret_cast<jlong>(ctx);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_CameraActivity_stopCameraNative(JNIEnv *env, jobject thiz, jlong context_ptr) {
    CameraContext* ctx = reinterpret_cast<CameraContext*>(context_ptr);
    stopCamera(ctx);
}
