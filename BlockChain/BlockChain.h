#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <vector>

#include "../Block/Block.h"


class BlockChain {
private:
    std::vector<Block> blockChain;
public:
    BlockChain();

    unsigned int size() const;
    bool empty() const;

    unsigned int getLastIndex() const;
    std::string getLastHash() const;

    void addBlock(const Block &block);

    std::string toString() const;

    Block& operator[](const unsigned int index);
    const Block& operator[](const unsigned int index) const;
};



#endif //BLOCKCHAIN_H