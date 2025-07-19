#include <android/native_activity.h>
#include <android/log.h>
#include <android/window.h>
#include <android/native_window_jni.h>

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraCaptureSession.h>
#include <media/NdkImageReader.h>

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "android_native_app_glue.h"

#define TAG "NDKCamera2"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

struct CameraApp {
    ACameraManager* cameraManager = nullptr;
    ACameraDevice* cameraDevice = nullptr;
    ACaptureSessionOutputContainer* outputContainer = nullptr;
    ACaptureSessionOutput* captureSessionOutput = nullptr;
    ACameraCaptureSession* captureSession = nullptr;
    AImageReader* imageReader = nullptr;
    ANativeWindow* nativeWindow = nullptr;
    ACaptureRequest* captureRequest = nullptr;
    ACameraOutputTarget* outputTarget = nullptr;

    ANativeActivity* activity = nullptr;
    std::atomic_bool running;

    std::mutex mutex;
    std::condition_variable cv;
};

static void onImageAvailable(void* context, AImageReader* reader) {
    AImage* image = nullptr;
    if (AImageReader_acquireNextImage(reader, &image) == AMEDIA_OK && image) {
        int32_t width, height, format;
        AImage_getWidth(image, &width);
        AImage_getHeight(image, &height);
        AImage_getFormat(image, &format);

        LOGI("Image acquired: %dx%d format=%d", width, height, format);

        // TODO: 这里可以处理YUV数据或者转码

        AImage_delete(image);
    }
}

static ACameraDevice_StateCallbacks cameraDeviceCallbacks = {
    .context = nullptr,
    .onDisconnected = [](void* ctx, ACameraDevice* device) {
        LOGI("Camera disconnected");
    },
    .onError = [](void* ctx, ACameraDevice* device, int err) {
        LOGE("Camera error: %d", err);
    }
};

static void onSessionClosed(void* ctx, ACameraCaptureSession* session) {
    LOGI("Capture session closed");
}

static void onSessionReady(void* ctx, ACameraCaptureSession* session) {
    LOGI("Capture session ready");
}

static void onSessionActive(void* ctx, ACameraCaptureSession* session) {
    LOGI("Capture session active");
}

static ACameraCaptureSession_stateCallbacks captureSessionCallbacks = {
    .context = nullptr,
    .onClosed = onSessionClosed,
    .onReady = onSessionReady,
    .onActive = onSessionActive
};

static void startCamera(CameraApp* app) {
    app->cameraManager = ACameraManager_create();
    ACameraIdList* cameraIdList = nullptr;
    if (ACameraManager_getCameraIdList(app->cameraManager, &cameraIdList) != ACAMERA_OK || cameraIdList->numCameras == 0) {
        LOGE("No cameras available");
        return;
    }
    const char* cameraId = cameraIdList->cameraIds[0];
    LOGI("Opening camera %s", cameraId);

    cameraDeviceCallbacks.context = app;
    if (ACameraManager_openCamera(app->cameraManager, cameraId, &cameraDeviceCallbacks, &app->cameraDevice) != ACAMERA_OK) {
        LOGE("Failed to open camera");
        return;
    }

    int width = 1280, height = 720;
    if (AImageReader_new(width, height, AIMAGE_FORMAT_YUV_420_888, 4, &app->imageReader) != AMEDIA_OK) {
        LOGE("Failed to create ImageReader");
        return;
    }
    if (AImageReader_getWindow(app->imageReader, &app->nativeWindow) != AMEDIA_OK) {
        LOGE("Failed to get native window");
        return;
    }

    AImageReader_ImageListener listener = {
        .context = app,
        .onImageAvailable = onImageAvailable
    };
    AImageReader_setImageListener(app->imageReader, &listener);

    if (ACaptureSessionOutputContainer_create(&app->outputContainer) != ACAMERA_OK) {
        LOGE("Failed to create output container");
        return;
    }
    if (ACaptureSessionOutput_create(app->nativeWindow, &app->captureSessionOutput) != ACAMERA_OK) {
        LOGE("Failed to create capture session output");
        return;
    }
    if (ACaptureSessionOutputContainer_add(app->outputContainer, app->captureSessionOutput) != ACAMERA_OK) {
        LOGE("Failed to add output");
        return;
    }

    captureSessionCallbacks.context = app;
    if (ACameraDevice_createCaptureSession(app->cameraDevice, app->outputContainer, &captureSessionCallbacks, &app->captureSession) != ACAMERA_OK) {
        LOGE("Failed to create capture session");
        return;
    }

    if (ACameraDevice_createCaptureRequest(app->cameraDevice, TEMPLATE_PREVIEW, &app->captureRequest) != ACAMERA_OK) {
        LOGE("Failed to create capture request");
        return;
    }
    if (ACameraOutputTarget_create(app->nativeWindow, &app->outputTarget) != ACAMERA_OK) {
        LOGE("Failed to create output target");
        return;
    }
    if (ACaptureRequest_addTarget(app->captureRequest, app->outputTarget) != ACAMERA_OK) {
        LOGE("Failed to add target");
        return;
    }
    if (ACameraCaptureSession_setRepeatingRequest(app->captureSession, nullptr, 1, &app->captureRequest, nullptr) != ACAMERA_OK) {
        LOGE("Failed to start preview");
        return;
    }

    LOGI("Camera preview started");
}

static void stopCamera(CameraApp* app) {
    if (!app) return;
    if (app->captureSession) {
        ACameraCaptureSession_stopRepeating(app->captureSession);
        ACameraCaptureSession_close(app->captureSession);
        app->captureSession = nullptr;
    }
    if (app->captureRequest) {
        ACaptureRequest_free(app->captureRequest);
        app->captureRequest = nullptr;
    }
    if (app->outputTarget) {
        ACameraOutputTarget_free(app->outputTarget);
        app->outputTarget = nullptr;
    }
    if (app->captureSessionOutput) {
        ACaptureSessionOutput_free(app->captureSessionOutput);
        app->captureSessionOutput = nullptr;
    }
    if (app->outputContainer) {
        ACaptureSessionOutputContainer_free(app->outputContainer);
        app->outputContainer = nullptr;
    }
    if (app->imageReader) {
        AImageReader_delete(app->imageReader);
        app->imageReader = nullptr;
    }
    if (app->cameraDevice) {
        ACameraDevice_close(app->cameraDevice);
        app->cameraDevice = nullptr;
    }
    if (app->cameraManager) {
        ACameraManager_delete(app->cameraManager);
        app->cameraManager = nullptr;
    }
    LOGI("Camera stopped and resources released");
}

static void handleAppCmd(struct android_app* app, int32_t cmd) {
    CameraApp* cameraApp = (CameraApp*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != nullptr && !cameraApp->running.load()) {
                cameraApp->running.store(true);
                // 启动摄像头
                std::thread cameraThread([cameraApp]() {
                    startCamera(cameraApp);
                });
                cameraThread.detach();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            cameraApp->running.store(false);
            stopCamera(cameraApp);
            break;
        case APP_CMD_DESTROY:
            cameraApp->running.store(false);
            stopCamera(cameraApp);
            break;
        default:
            break;
    }
}

void android_main(struct android_app* state) {
    //app_dummy(); // 防止静态链接丢失

    CameraApp cameraApp{};
    cameraApp.activity = state->activity;
    cameraApp.running = false;

    state->userData = &cameraApp;
    state->onAppCmd = handleAppCmd;

    int events;
    android_poll_source* source;

    while (true) {
        while (ALooper_pollAll(cameraApp.running ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(state, source);
            }

            if (state->destroyRequested) {
                stopCamera(&cameraApp);
                return;
            }
        }
    }
}
