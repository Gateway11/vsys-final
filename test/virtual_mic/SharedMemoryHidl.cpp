#include <android/hardware/sharedmemory/1.0/ISharedMemoryExample.h>
#include <hidlmemory/mapping.h>
#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

using android::hardware::sharedmemory::V1_0::ISharedMemoryExample;
using android::hardware::hidl_memory;
using android::sp;

class SharedMemoryExample : public ISharedMemoryExample {
public:
    Return<bool> transferDataFromSharedMemory(hidl_memory memory) override {
        // Map the passed hidl_memory to a void* pointer
        sp<IMemory> mappedMemory = mapMemory(memory);
        if (mappedMemory == nullptr) {
            return false; // Mapping failed
        }

        // Lock memory access
        mappedMemory->update();

        // Get the shared memory pointer and read data
        void* data = static_cast<void*>(mappedMemory->getPointer());
        size_t size = mappedMemory->getSize();

        if (data == nullptr || size == 0) {
            return false; // Invalid memory
        }

        // Read and process the data in the shared memory
        // For example, print the data
        char* charData = static_cast<char*>(data);
        std::string receivedData(charData, size);
        printf("Received data: %s\n", receivedData.c_str());

        // Unlock memory
        mappedMemory->commit();

        return true;
    }
};

#include <android/hardware/sharedmemory/1.0/ISharedMemoryExample.h>
#include <hidlmemory/mapping.h>
#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

using android::hardware::sharedmemory::V1_0::ISharedMemoryExample;
using android::hardware::hidl_memory;
using android::sp;
using android::hardware::hidl_handle;
using android::hardware::createMemory;

int main() {
    // Get the service
    sp<ISharedMemoryExample> service = ISharedMemoryExample::getService();
    if (service == nullptr) {
        printf("Failed to get service!\n");
        return -1;
    }

    // Create shared memory once (1 KB in size)
    size_t memorySize = 1024;
    sp<IMemory> memory = mapMemory(createMemory("ashmem", memorySize));

    if (memory == nullptr) {
        printf("Failed to create shared memory!\n");
        return -1;
    }

    // Use the same shared memory multiple times to send data
    for (int i = 0; i < 3; ++i) {
        // Write different data into the shared memory
        std::string data = "Hello from Client, message " + std::to_string(i + 1);
        memory->update();
        void* memPointer = static_cast<void*>(memory->getPointer());

        if (memPointer != nullptr) {
            memcpy(memPointer, data.c_str(), data.length() + 1);  // Copy new data to shared memory
            memory->commit();  // Commit the changes
        } else {
            printf("Memory pointer is null!\n");
            return -1;
        }

        // Create hidl_memory object and pass it to the server
        hidl_memory hidlMem = memory->getMemory();

        bool result = service->transferDataFromSharedMemory(hidlMem);
        if (result) {
            printf("Data transferred successfully for message %d!\n", i + 1);
        } else {
            printf("Failed to transfer data for message %d!\n", i + 1);
        }
    }

    return 0;
}
// ####################################################################################
http://aospxref.com/android-14.0.0_r2/xref/frameworks/av/media/libaudiohal/impl/EffectBufferHalHidl.cpp
status_t EffectBufferHalHidl::init() {
    sp<IAllocator> ashmem = IAllocator::getService("ashmem");
    if (ashmem == 0) {
        ALOGE("Failed to retrieve ashmem allocator service");
        return NO_INIT;
    }
    status_t retval = NO_MEMORY;
    Return<void> result = ashmem->allocate(
            mBufferSize,
            [&](bool success, const hidl_memory& memory) {
                if (success) {
                    mHidlBuffer.data = memory;
                    retval = OK;
                }
            });
    if (result.isOk() && retval == OK) {
        mMemory = hardware::mapMemory(mHidlBuffer.data);
        if (mMemory != 0) {
            mMemory->update();
            mAudioBuffer.raw = static_cast<void*>(mMemory->getPointer());
            memset(mAudioBuffer.raw, 0, mMemory->getSize());
            mMemory->commit();
        } else {
            ALOGE("Failed to map allocated ashmem");
            retval = NO_MEMORY;
        }
    } else {
        ALOGE("Failed to allocate %d bytes from ashmem", (int)mBufferSize);
    }
    return result.isOk() ? retval : FAILED_TRANSACTION;
}
// ####################################################################################

//1
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <hidlmemory/HidlMemoryDealer.h>
#include <android/hidl/memory/block/1.0/types.h>
#include <hidlmemory/mapping.h>

using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hardware::hidl_memory;
using ::android::hardware::HidlMemoryDealer
using ::android::hidl::memory::block::V1_0::MemoryBlock;

sp<IAllocator> allocator = IAllocator::getService("ashmem");
allocator->allocate(2048, [&](bool success, const hidl_memory& mem)
{
    if (!success) { /* error */ }
    // you can now use the hidl_memory object 'mem' or pass it
}));

sp<HidlMemoryDealer> memory_dealer = HidlMemoryDealer::getInstance(mem);
Return<void> Foo::getSome(getSome_cb _hidl_cb) {
    MemoryBlock block = memory_dealer->allocate(1024);
    if(HidlMemoryDealer::isOk(block)){
        _hidl_cb(block);
}

Return<void> Foo::giveBack(const MemoryBlock& block) {
    memory_dealer->deallocate(block.offset);
}

////////////////////////////////////////////////////////////////////////////////////////

#include <hidlmemory/mapping.h>
#include <android/hidl/memory/1.0/IMemory.h>

using ::android::hidl::memory::V1_0::IMemory;

sp<RefBase> lockMemory(block.token);
sp<IMemory> memory = mapMemory(block);
uint8_t* data = static_cast<uint8_t*>(static_cast<void*>(memory->getPointer()));
