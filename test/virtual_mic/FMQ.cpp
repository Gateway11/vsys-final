#define LOG_TAG "StreamOutHAL"

#include "core/default/StreamOut.h"
#include "core/default/Util.h"

//#define LOG_NDEBUG 0
#define ATRACE_TAG ATRACE_TAG_AUDIO

#include <string.h>

#include <memory>

#include <HidlUtils.h>
#include <android/log.h>
#include <audio_utils/Metadata.h>
#include <hardware/audio.h>
#include <util/CoreUtils.h>
#include <utils/Trace.h>

namespace android {
namespace hardware {
namespace audio {
namespace CPP_VERSION {
namespace implementation {

namespace {

class WriteThread : public Thread {
   public:
    // WriteThread's lifespan never exceeds StreamOut's lifespan.
    WriteThread(std::atomic<bool>* stop, audio_stream_out_t* stream,
                StreamOut::CommandMQ* commandMQ, StreamOut::DataMQ* dataMQ,
                StreamOut::StatusMQ* statusMQ, EventFlag* efGroup)
        : Thread(false /*canCallJava*/),
          mStop(stop),
          mStream(stream),
          mCommandMQ(commandMQ),
          mDataMQ(dataMQ),
          mStatusMQ(statusMQ),
          mEfGroup(efGroup),
          mBuffer(nullptr) {}
    bool init() {
        mBuffer.reset(new (std::nothrow) uint8_t[mDataMQ->getQuantumCount()]);
        return mBuffer != nullptr;
    }
    virtual ~WriteThread() {}

   private:
    std::atomic<bool>* mStop;
    audio_stream_out_t* mStream;
    StreamOut::CommandMQ* mCommandMQ;
    StreamOut::DataMQ* mDataMQ;
    StreamOut::StatusMQ* mStatusMQ;
    EventFlag* mEfGroup;
    std::unique_ptr<uint8_t[]> mBuffer;
    IStreamOut::WriteStatus mStatus;

    bool threadLoop() override;

    void doGetLatency();
    void doGetPresentationPosition();
    void doWrite();
};

void WriteThread::doWrite() {
    const size_t availToRead = mDataMQ->availableToRead();
    mStatus.retval = Result::OK;
    mStatus.reply.written = 0;
    if (mDataMQ->read(&mBuffer[0], availToRead)) {
        ssize_t writeResult = mStream->write(mStream, &mBuffer[0], availToRead);
        if (writeResult >= 0) {
            mStatus.reply.written = writeResult;
        } else {
            mStatus.retval = Stream::analyzeStatus("write", writeResult);
        }
    }
}

void WriteThread::doGetPresentationPosition() {
    mStatus.retval =
        StreamOut::getPresentationPositionImpl(mStream, &mStatus.reply.presentationPosition.frames,
                                               &mStatus.reply.presentationPosition.timeStamp);
}

void WriteThread::doGetLatency() {
    mStatus.retval = Result::OK;
    mStatus.reply.latencyMs = mStream->get_latency(mStream);
}

bool WriteThread::threadLoop() {
    // This implementation doesn't return control back to the Thread until it
    // decides to stop,
    // as the Thread uses mutexes, and this can lead to priority inversion.
    while (!std::atomic_load_explicit(mStop, std::memory_order_acquire)) {
        uint32_t efState = 0;
        mEfGroup->wait(static_cast<uint32_t>(MessageQueueFlagBits::NOT_EMPTY), &efState);
        if (!(efState & static_cast<uint32_t>(MessageQueueFlagBits::NOT_EMPTY))) {
            continue;  // Nothing to do.
        }
        if (!mCommandMQ->read(&mStatus.replyTo)) {
            continue;  // Nothing to do.
        }
        switch (mStatus.replyTo) {
            case IStreamOut::WriteCommand::WRITE:
                doWrite();
                break;
            case IStreamOut::WriteCommand::GET_PRESENTATION_POSITION:
                doGetPresentationPosition();
                break;
            case IStreamOut::WriteCommand::GET_LATENCY:
                doGetLatency();
                break;
            default:
                ALOGE("Unknown write thread command code %d", mStatus.replyTo);
                mStatus.retval = Result::NOT_SUPPORTED;
                break;
        }
        if (!mStatusMQ->write(&mStatus)) {
            ALOGE("status message queue write failed");
        }
        mEfGroup->wake(static_cast<uint32_t>(MessageQueueFlagBits::NOT_FULL));
    }

    return false;
}

}  // namespace

Return<void> StreamOut::prepareForWriting(uint32_t frameSize, uint32_t framesCount,
                                          prepareForWriting_cb _hidl_cb) {
    status_t status;
#if MAJOR_VERSION <= 6
    ThreadInfo threadInfo = {0, 0};
#else
    int32_t threadInfo = 0;
#endif

    // Wrap the _hidl_cb to return an error
    auto sendError = [&threadInfo, &_hidl_cb](Result result) {
        _hidl_cb(result, CommandMQ::Descriptor(), DataMQ::Descriptor(), StatusMQ::Descriptor(),
                 threadInfo);
    };

    // Create message queues.
    if (mDataMQ) {
        ALOGE("the client attempts to call prepareForWriting twice");
        sendError(Result::INVALID_STATE);
        return Void();
    }
    std::unique_ptr<CommandMQ> tempCommandMQ(new CommandMQ(1));

    // Check frameSize and framesCount
    if (frameSize == 0 || framesCount == 0) {
        ALOGE("Null frameSize (%u) or framesCount (%u)", frameSize, framesCount);
        sendError(Result::INVALID_ARGUMENTS);
        return Void();
    }
    if (frameSize > Stream::MAX_BUFFER_SIZE / framesCount) {
        ALOGE("Buffer too big: %u*%u bytes > MAX_BUFFER_SIZE (%u)", frameSize, framesCount,
              Stream::MAX_BUFFER_SIZE);
        sendError(Result::INVALID_ARGUMENTS);
        return Void();
    }
    std::unique_ptr<DataMQ> tempDataMQ(new DataMQ(frameSize * framesCount, true /* EventFlag */));

    std::unique_ptr<StatusMQ> tempStatusMQ(new StatusMQ(1));
    if (!tempCommandMQ->isValid() || !tempDataMQ->isValid() || !tempStatusMQ->isValid()) {
        ALOGE_IF(!tempCommandMQ->isValid(), "command MQ is invalid");
        ALOGE_IF(!tempDataMQ->isValid(), "data MQ is invalid");
        ALOGE_IF(!tempStatusMQ->isValid(), "status MQ is invalid");
        sendError(Result::INVALID_ARGUMENTS);
        return Void();
    }
    EventFlag* tempRawEfGroup{};
    status = EventFlag::createEventFlag(tempDataMQ->getEventFlagWord(), &tempRawEfGroup);
    std::unique_ptr<EventFlag, void (*)(EventFlag*)> tempElfGroup(
        tempRawEfGroup, [](auto* ef) { EventFlag::deleteEventFlag(&ef); });
    if (status != OK || !tempElfGroup) {
        ALOGE("failed creating event flag for data MQ: %s", strerror(-status));
        sendError(Result::INVALID_ARGUMENTS);
        return Void();
    }

    // Create and launch the thread.
    auto tempWriteThread =
            sp<WriteThread>::make(&mStopWriteThread, mStream, tempCommandMQ.get(), tempDataMQ.get(),
                                  tempStatusMQ.get(), tempElfGroup.get());
    if (!tempWriteThread->init()) {
        ALOGW("failed to start writer thread: %s", strerror(-status));
        sendError(Result::INVALID_ARGUMENTS);
        return Void();
    }
    status = tempWriteThread->run("writer", PRIORITY_URGENT_AUDIO);
    if (status != OK) {
        ALOGW("failed to start writer thread: %s", strerror(-status));
        sendError(Result::INVALID_ARGUMENTS);
        return Void();
    }

    mCommandMQ = std::move(tempCommandMQ);
    mDataMQ = std::move(tempDataMQ);
    mStatusMQ = std::move(tempStatusMQ);
    mWriteThread = tempWriteThread;
    mEfGroup = tempElfGroup.release();
#if MAJOR_VERSION <= 6
    threadInfo.pid = getpid();
    threadInfo.tid = mWriteThread->getTid();
#else
    threadInfo = mWriteThread->getTid();
#endif
    _hidl_cb(Result::OK, *mCommandMQ->getDesc(), *mDataMQ->getDesc(), *mStatusMQ->getDesc(),
             threadInfo);
    return Void();
}

}  // namespace implementation
}  // namespace CPP_VERSION
}  // namespace audio
}  // namespace hardware
}  // namespace android


///////////////////////////////////// client /////////////////////////////////////////

#define LOG_TAG "StreamHalHidl"
//#define LOG_NDEBUG 0

#include <android/hidl/manager/1.0/IServiceManager.h>
#include <hwbinder/IPCThreadState.h>
#include <media/AudioParameter.h>
#include <mediautils/memory.h>
#include <mediautils/SchedulingPolicyService.h>
#include <mediautils/TimeCheck.h>
#include <utils/Log.h>
#if MAJOR_VERSION >= 4
#include <media/AidlConversion.h>
#endif

status_t StreamOutHalHidl::write(const void *buffer, size_t bytes, size_t *written) {
    // TIME_CHECK();  // TODO(b/243839867) reenable only when optimized.
    if (mStream == 0) return NO_INIT;
    *written = 0;

    if (bytes == 0 && !mDataMQ) {
        // Can't determine the size for the MQ buffer. Wait for a non-empty write request.
        ALOGW_IF(mCallback.load().unsafe_get(), "First call to async write with 0 bytes");
        return OK;
    }

    status_t status;
    if (!mDataMQ) {
        // In case if playback starts close to the end of a compressed track, the bytes
        // that need to be written is less than the actual buffer size. Need to use
        // full buffer size for the MQ since otherwise after seeking back to the middle
        // data will be truncated.
        size_t bufferSize;
        if ((status = getCachedBufferSize(&bufferSize)) != OK) {
            return status;
        }
        if (bytes > bufferSize) bufferSize = bytes;
        if ((status = prepareForWriting(bufferSize)) != OK) {
            return status;
        }
    }

    status = callWriterThread(
            WriteCommand::WRITE, "write", static_cast<const uint8_t*>(buffer), bytes,
            [&] (const WriteStatus& writeStatus) {
                *written = writeStatus.reply.written;
                // Diagnostics of the cause of b/35813113.
                ALOGE_IF(*written > bytes,
                        "hal reports more bytes written than asked for: %lld > %lld",
                        (long long)*written, (long long)bytes);
            });
    mStreamPowerLog.log(buffer, *written);
    return status;
}

status_t StreamOutHalHidl::callWriterThread(
        WriteCommand cmd, const char* cmdName,
        const uint8_t* data, size_t dataSize, StreamOutHalHidl::WriterCallback callback) {
    if (!mCommandMQ->write(&cmd)) {
        ALOGE("command message queue write failed for \"%s\"", cmdName);
        return -EAGAIN;
    }
    if (data != nullptr) {
        size_t availableToWrite = mDataMQ->availableToWrite();
        if (dataSize > availableToWrite) {
            ALOGW("truncating write data from %lld to %lld due to insufficient data queue space",
                    (long long)dataSize, (long long)availableToWrite);
            dataSize = availableToWrite;
        }
        if (!mDataMQ->write(data, dataSize)) {
            ALOGE("data message queue write failed for \"%s\"", cmdName);
        }
    }
    mEfGroup->wake(static_cast<uint32_t>(MessageQueueFlagBits::NOT_EMPTY));

    // TODO: Remove manual event flag handling once blocking MQ is implemented. b/33815422
    uint32_t efState = 0;
retry:
    status_t ret = mEfGroup->wait(static_cast<uint32_t>(MessageQueueFlagBits::NOT_FULL), &efState);
    if (efState & static_cast<uint32_t>(MessageQueueFlagBits::NOT_FULL)) {
        WriteStatus writeStatus;
        writeStatus.retval = Result::NOT_INITIALIZED;
        if (!mStatusMQ->read(&writeStatus)) {
            ALOGE("status message read failed for \"%s\"", cmdName);
        }
        if (writeStatus.retval == Result::OK) {
            ret = OK;
            callback(writeStatus);
        } else {
            ret = processReturn(cmdName, writeStatus.retval);
        }
        return ret;
    }
    if (ret == -EAGAIN || ret == -EINTR) {
        // Spurious wakeup. This normally retries no more than once.
        goto retry;
    }
    return ret;
}

status_t StreamOutHalHidl::prepareForWriting(size_t bufferSize) {
    std::unique_ptr<CommandMQ> tempCommandMQ;
    std::unique_ptr<DataMQ> tempDataMQ;
    std::unique_ptr<StatusMQ> tempStatusMQ;
    Result retval;
    pid_t halThreadPid, halThreadTid;
    Return<void> ret = mStream->prepareForWriting(
            1, bufferSize,
            [&](Result r,
                    const CommandMQ::Descriptor& commandMQ,
                    const DataMQ::Descriptor& dataMQ,
                    const StatusMQ::Descriptor& statusMQ,
                    const auto& halThreadInfo) {
                retval = r;
                if (retval == Result::OK) {
                    tempCommandMQ.reset(new CommandMQ(commandMQ));
                    tempDataMQ.reset(new DataMQ(dataMQ));
                    tempStatusMQ.reset(new StatusMQ(statusMQ));
                    if (tempDataMQ->isValid() && tempDataMQ->getEventFlagWord()) {
                        EventFlag::createEventFlag(tempDataMQ->getEventFlagWord(), &mEfGroup);
                    }
#if MAJOR_VERSION <= 6
                    halThreadPid = halThreadInfo.pid;
                    halThreadTid = halThreadInfo.tid;
#else
                    halThreadTid = halThreadInfo;
#endif
                }
            });
    if (!ret.isOk() || retval != Result::OK) {
        return processReturn("prepareForWriting", ret, retval);
    }
    if (!tempCommandMQ || !tempCommandMQ->isValid() ||
            !tempDataMQ || !tempDataMQ->isValid() ||
            !tempStatusMQ || !tempStatusMQ->isValid() ||
            !mEfGroup) {
        ALOGE_IF(!tempCommandMQ, "Failed to obtain command message queue for writing");
        ALOGE_IF(tempCommandMQ && !tempCommandMQ->isValid(),
                "Command message queue for writing is invalid");
        ALOGE_IF(!tempDataMQ, "Failed to obtain data message queue for writing");
        ALOGE_IF(tempDataMQ && !tempDataMQ->isValid(), "Data message queue for writing is invalid");
        ALOGE_IF(!tempStatusMQ, "Failed to obtain status message queue for writing");
        ALOGE_IF(tempStatusMQ && !tempStatusMQ->isValid(),
                "Status message queue for writing is invalid");
        ALOGE_IF(!mEfGroup, "Event flag creation for writing failed");
        return NO_INIT;
    }
#if MAJOR_VERSION >= 7
    if (status_t status = getHalPid(&halThreadPid); status != NO_ERROR) {
        return status;
    }
#endif
    requestHalThreadPriority(halThreadPid, halThreadTid);

    mCommandMQ = std::move(tempCommandMQ);
    mDataMQ = std::move(tempDataMQ);
    mStatusMQ = std::move(tempStatusMQ);
    mWriterClient = gettid();
    return OK;
}

} // namespace android
