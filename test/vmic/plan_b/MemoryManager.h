#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <iostream>
#include <vector>
#include <cstring>

class MemoryManager {
private:
    char* memory;                        // Pointer to the dynamically allocated memory
    size_t totalSize;                    // Total size of the memory
    size_t blockSize;                    // Size of each memory block
    size_t numBlocks;                    // Total number of memory blocks
    std::vector<bool> blockUsage;        // Tracks the usage of each block

public:
    MemoryManager(size_t totalSize, size_t blockSize)
        : totalSize(totalSize), blockSize(blockSize) {

        memory = new char[totalSize];

        numBlocks = totalSize / blockSize;
        blockUsage.resize(numBlocks, false);
    }

    ~MemoryManager() {
        delete[] memory;  // Free the allocated memory
    }

    void* allocate() {
        // Find the first unused block
        for (size_t i = 0; i < numBlocks; ++i) {
            if (!blockUsage[i]) {
                blockUsage[i] = true;  // Mark the block as used
                return memory + i * blockSize;  // Return the starting address of the block
            }
        }

        // If no block is available, return nullptr
        return nullptr;
    }

    void deallocate(void* ptr) {
        // Calculate the index of the block
        size_t index = (static_cast<char*>(ptr) - memory) / blockSize;

        // Check if the index is valid
        if (index < numBlocks) {
            blockUsage[index] = false;  // Mark the block as unused
        }
    }

    void displayUsage() const {
        // Display the usage status of each block
        std::cout << "Memory block usage:\n";
        for (size_t i = 0; i < numBlocks; ++i) {
            std::cout << "Block " << i << ": " << (blockUsage[i] ? "Used" : "Free") << std::endl;
        }
    }
};

#endif // MEMORYMANAGER_H

#if 0
int main() {
    // Allocate 2MB of memory and divide it into blocks of 3840 bytes
    size_t totalMemorySize = 2 * 1024 * 1024;  // 2MB
    size_t blockSize = 3840;  // Block size of 3840 bytes

    MemoryManager manager(totalMemorySize, blockSize);

    // Allocate a few memory blocks
    void* block1 = manager.allocate();
    void* block2 = manager.allocate();

    // Display memory usage
    manager.displayUsage();

    // Deallocate the first block
    manager.deallocate(block1);

    // Display memory usage again
    manager.displayUsage();

    return 0;
}
#endif
