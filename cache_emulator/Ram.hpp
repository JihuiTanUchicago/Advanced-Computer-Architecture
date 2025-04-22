#ifndef RAM_HPP
#define RAM_HPP

#include "DataBlock.hpp"
#include "Address.hpp"
#include <vector>
#include <stdexcept>

class Ram {
private:
    int numBlocks;
    int blockSize;
    std::vector<DataBlock> data;

public:
    Ram(int totalBytes, int blockSize) : blockSize(blockSize) {
        numBlocks = totalBytes / blockSize;
        for (int i = 0; i < numBlocks; ++i) {
            data.emplace_back(blockSize);
        }
    }

    DataBlock getBlock(const Address& addr) const {
        int blockIndex = addr.getRawAddress() / blockSize;
        if (blockIndex < 0 || blockIndex >= numBlocks) {
            throw std::out_of_range("RAM access out of range");
        }
        return data[blockIndex];
    }

    void setBlock(const Address& addr, const DataBlock& block) {
        int blockIndex = addr.getRawAddress() / blockSize;
        if (blockIndex < 0 || blockIndex >= numBlocks) {
            throw std::out_of_range("RAM write out of range");
        }
        data[blockIndex].loadFrom(block);
    }

    int getSize() const {
        return numBlocks * blockSize;
    }
};

#endif