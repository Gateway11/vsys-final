#include <android/log.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraCaptureSession.h>
#include <media/NdkImageReader.h>
#include <android/native_window.h>

#define TAG "CameraNDK"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

struct CameraContext {
    ACameraDevice* cameraDevice = nullptr;
    ACaptureSessionOutputContainer* outputContainer = nullptr;
    ACaptureSessionOutput* captureSessionOutput = nullptr;
    ACameraCaptureSession* captureSession = nullptr;
    AImageReader* imageReader = nullptr;
    ANativeWindow* nativeWindow = nullptr;
    ACaptureRequest* captureRequest = nullptr;
    ACameraOutputTarget* outputTarget = nullptr;
};

static void onImageAvailable(void* context, AImageReader* reader) {
    AImage* image = nullptr;
    if (AImageReader_acquireNextImage(reader, &image) == AMEDIA_OK && image) {
        int32_t width = 0, height = 0, format = 0;
        AImage_getWidth(image, &width);
        AImage_getHeight(image, &height);
        AImage_getFormat(image, &format);
        LOGI("Image acquired: width=%d height=%d format=%d", width, height, format);

        // 这里可以访问 YUV 或其他格式平面数据...

        AImage_delete(image);
    }
}

static ACameraDevice_StateCallbacks cameraDeviceCallbacks = {
    .context = nullptr,
    .onDisconnected = [](void* context, ACameraDevice* device) {
        LOGI("Camera disconnected");
    },
    .onError = [](void* context, ACameraDevice* device, int error) {
        LOGE("Camera error: %d", error);
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

void startCamera() {
    CameraContext* ctx = new CameraContext();

    ACameraManager* cameraManager = ACameraManager_create();

    ACameraIdList* cameraIdList = nullptr;
    if (ACameraManager_getCameraIdList(cameraManager, &cameraIdList) != ACAMERA_OK || cameraIdList->numCameras == 0) {
        LOGE("No camera available");
        return;
    }

    const char* cameraId = cameraIdList->cameraIds[0];
    LOGI("Opening camera %s", cameraId);

    cameraDeviceCallbacks.context = ctx;
    if (ACameraManager_openCamera(cameraManager, cameraId, &cameraDeviceCallbacks, &ctx->cameraDevice) != ACAMERA_OK) {
        LOGE("Failed to open camera");
        return;
    }

    // 创建 ImageReader
    int width = 1280;
    int height = 720;
    int maxImages = 4;
    if (AImageReader_new(width, height, AIMAGE_FORMAT_YUV_420_888, maxImages, &ctx->imageReader) != AMEDIA_OK) {
        LOGE("Failed to create ImageReader");
        return;
    }

    if (AImageReader_getWindow(ctx->imageReader, &ctx->nativeWindow) != AMEDIA_OK) {
        LOGE("Failed to get native window from ImageReader");
        return;
    }

    // 设置图像监听器
    AImageReader_ImageListener imageListener = {
        .context = ctx,
        .onImageAvailable = onImageAvailable,
    };
    AImageReader_setImageListener(ctx->imageReader, &imageListener);

    // 创建 CaptureSessionOutputContainer 和 CaptureSessionOutput
    if (ACaptureSessionOutputContainer_create(&ctx->outputContainer) != ACAMERA_OK) {
        LOGE("Failed to create CaptureSessionOutputContainer");
        return;
    }

    if (ACaptureSessionOutput_create(ctx->nativeWindow, &ctx->captureSessionOutput) != ACAMERA_OK) {
        LOGE("Failed to create CaptureSessionOutput");
        return;
    }

    if (ACaptureSessionOutputContainer_add(ctx->outputContainer, ctx->captureSessionOutput) != ACAMERA_OK) {
        LOGE("Failed to add CaptureSessionOutput");
        return;
    }

    // 创建 CaptureSession
    captureSessionCallbacks.context = ctx;
    if (ACameraDevice_createCaptureSession(ctx->cameraDevice, ctx->outputContainer, &captureSessionCallbacks, &ctx->captureSession) != ACAMERA_OK) {
        LOGE("Failed to create capture session");
        return;
    }

    // 创建 CaptureRequest
    if (ACameraDevice_createCaptureRequest(ctx->cameraDevice, TEMPLATE_PREVIEW, &ctx->captureRequest) != ACAMERA_OK) {
        LOGE("Failed to create capture request");
        return;
    }

    // 创建输出目标
    if (ACameraOutputTarget_create(ctx->nativeWindow, &ctx->outputTarget) != ACAMERA_OK) {
        LOGE("Failed to create output target");
        return;
    }

    if (ACaptureRequest_addTarget(ctx->captureRequest, ctx->outputTarget) != ACAMERA_OK) {
        LOGE("Failed to add target to capture request");
        return;
    }

    // 提交重复请求
    if (ACameraCaptureSession_setRepeatingRequest(ctx->captureSession, nullptr, 1, &ctx->captureRequest, nullptr) != ACAMERA_OK) {
        LOGE("Failed to set repeating request");
        return;
    }

    LOGI("Camera preview started");
}
