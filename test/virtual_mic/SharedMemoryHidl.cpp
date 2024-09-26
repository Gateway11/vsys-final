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
    Return<bool> transferDataFromSharedMemory(const hidl_memory& memory) override {
        // Map the passed hidl_memory to a void* pointer
        sp<IMemory> mappedMemory = mapMemory(memory);
        if (mappedMemory == nullptr) {
            return false; // Mapping failed
        }

#if 1
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
#else
        mappedMemory->read();
        ALOGV("Received data: %s\n", static_cast<char*>(mappedMemory->getPointer())));
#endif
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
    sp<IMemory> memory;
    bool success = false;
    Return<void> result = ashmem->allocate(memorySize, [&](bool s, const hidl_memory& m) {
        success = s;
        if (success) memory = mapMemory(m);
    });
    //sp<IMemory> memory = mapMemory(createMemory("ashmem", memorySize));
    if (!success && memory == nullptr) {
        printf("Failed to allocate %d bytes from ashmem\n", memorySize);
        return -1;
    }

    //hardware/interfaces/soundtrigger/2.1/default/SoundTriggerHw.cpp
    for (int i = 0; i < 3; ++i) {
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
// ####################################################################################

// Backs up by the vector with the contents of shared memory.
// It is assumed that the passed hidl_vector is empty, so it's
// not cleared if the memory is a null object.
// The caller needs to keep the returned sp<IMemory> as long as
// the data is needed.
std::pair<bool, sp<IMemory>> memoryAsVector(const hidl_memory& m, hidl_vec<uint8_t>* vec) {
    sp<IMemory> memory;
    if (m.size() == 0) {
        return std::make_pair(true, memory);
    }
    memory = mapMemory(m);
    if (memory != nullptr) {
        memory->read();
        vec->setToExternal(static_cast<uint8_t*>(static_cast<void*>(memory->getPointer())),
                           memory->getSize());
        return std::make_pair(true, memory);
    }
    ALOGE("%s: Could not map HIDL memory to IMemory", __func__);
    return std::make_pair(false, memory);
}

// Moves the data from the vector into allocated shared memory,
// emptying the vector.
// It is assumed that the passed hidl_memory is a null object, so it's
// not reset if the vector is empty.
// The caller needs to keep the returned sp<IMemory> as long as
// the data is needed.
std::pair<bool, sp<IMemory>> moveVectorToMemory(hidl_vec<uint8_t>* v, hidl_memory* mem) {
    sp<IMemory> memory;
    if (v->size() == 0) {
        return std::make_pair(true, memory);
    }
    sp<IAllocator> ashmem = IAllocator::getService("ashmem");
    if (ashmem == 0) {
        ALOGE("Failed to retrieve ashmem allocator service");
        return std::make_pair(false, memory);
    }
    bool success = false;
    Return<void> r = ashmem->allocate(v->size(), [&](bool s, const hidl_memory& m) {
        success = s;
        if (success) *mem = m;
    });
    if (r.isOk() && success) {
        memory = hardware::mapMemory(*mem);
        if (memory != 0) {
            memory->update();
            memcpy(memory->getPointer(), v->data(), v->size());
            memory->commit();
            v->resize(0);
            return std::make_pair(true, memory);
        } else {
            ALOGE("Failed to map allocated ashmem");
        }
    } else {
        ALOGE("Failed to allocate %llu bytes from ashmem", (unsigned long long)v->size());
    }
    return std::make_pair(false, memory);
}

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
