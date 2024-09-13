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
