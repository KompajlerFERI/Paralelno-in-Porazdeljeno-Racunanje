#include "BlockChain.h"

#include <stdexcept>

BlockChain::BlockChain(): blockChain(std::vector<Block>()) {}


unsigned int BlockChain::size() const {
    return blockChain.size();
}

bool BlockChain::empty() const {
    return blockChain.empty();
}


unsigned int BlockChain::getLastIndex() const {
    if (!blockChain.empty()) return blockChain[blockChain.size() - 1].index;
    throw std::out_of_range("blockChain is empty");
}

std::string BlockChain::getLastHash() const {
    if (!blockChain.empty()) return blockChain[blockChain.size() - 1].hash;
    throw std::out_of_range("blockChain is empty");
}


void BlockChain::addBlock(const Block &block) {
    blockChain.emplace_back(block);
}


std::string BlockChain::toString() const {
    std::string blockChainString;
    for (const Block &block: blockChain) {
        blockChainString += block.toString();
    }
    return blockChainString;
}


Block &BlockChain::operator[](const unsigned int index) {
    if (index < blockChain.size()) return blockChain[index];
    throw std::out_of_range("Index out of range");
}

const Block &BlockChain::operator[](const unsigned int index) const {
    if (index < blockChain.size()) return blockChain[index];
    throw std::out_of_range("Index out of range");
}


