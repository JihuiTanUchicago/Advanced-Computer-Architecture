#ifndef DATABLOCK_HPP
#define DATABLOCK_HPP

#include <vector>

class DataBlock {
private:
    std::vector<double> data;
    bool valid;
    int tag;
    int blockSize;

public:
    DataBlock(int blockSize) : valid(false), tag(-1), blockSize(blockSize) {
        data.resize(blockSize / sizeof(double), 0.0);
    }

    double get(int offset) const {
        return data[offset];
    }

    void set(int offset, double value) {
        data[offset] = value;
    }

    bool isValid() const {
        return valid;
    }

    void setValid(bool v) {
        valid = v;
    }

    int getTag() const {
        return tag;
    }

    void setTag(int t) {
        tag = t;
    }

    std::vector<double>& getRawData() {
        return data;
    }

    void loadFrom(const DataBlock& other) {
        data = other.data;
        tag = other.tag;
        valid = other.valid;
    }
};

#endif