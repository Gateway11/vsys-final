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
        ALOGV("Received data: %s\n", receivedData.c_str());

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
    sp<IAllocator> ashmem = IAllocator::getService("ashmem");
    if (ashmem == 0) {
        printf("Failed to retrieve ashmem allocator service\n");
        return -1;
    }
    //hardware/interfaces/soundtrigger/2.1/default/SoundTriggerHw.cpp
    bool success = false;
    hidl_memory mem;
    Return<void> r = ashmem->allocate(memorySize, [&](bool s, const hidl_memory& m) {
        success = s;
        if (success) mem = m;
    });
    if (r.isOk() && success) {
        sp<IMemory> memory = mapMemory(mem);
        if (memory != 0) {
            std::string data = "Hello from Client, message " + std::to_string(1);
            memory->update();
            memcpy(memory->getPointer(), data.c_str(), data.length());
            memory->commit();
            bool result = service->transferDataFromSharedMemory(hidlMem);
        } else {
            printf("Failed to map allocated ashmem\n");
        }
    } else {
        printf("Failed to allocate %d bytes from ashmem\n", memorySize);
    }
    return 0;
}
// ####################################################################################
http://aospxref.com/android-14.0.0_r2/xref/frameworks/av/media/libaudiohal/impl/EffectBufferHalHidl.cpp
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
