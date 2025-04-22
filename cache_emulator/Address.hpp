#ifndef ADDRESS_HPP
#define ADDRESS_HPP

class Address {
private:
    unsigned int address;
    int offsetBits, indexBits;
    unsigned int offsetMask, indexMask;

public:
    Address(unsigned int addr, int blockSize, int numSets) : address(addr) {
        offsetBits = __builtin_ctz(blockSize);
        indexBits = __builtin_ctz(numSets);
        offsetMask = (1 << offsetBits) - 1;
        indexMask = ((1 << indexBits) - 1) << offsetBits;
    }

    int getOffset() const {
        return address & offsetMask;
    }

    int getIndex() const {
        return (address & indexMask) >> offsetBits;
    }

    int getTag() const {
        return address >> (offsetBits + indexBits);
    }

    unsigned int getRawAddress() const {
        return address;
    }
};

#endif