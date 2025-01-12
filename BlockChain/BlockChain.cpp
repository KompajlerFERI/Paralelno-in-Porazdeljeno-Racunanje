#include "BlockChain.h"

#include <iostream>
#include <stdexcept>
#include <cmath>

#include "../sha256/sha256.h"

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

bool BlockChain::isValid() const {
    if (empty()) return false;

    // V A L I D A T E   F I R S T   B L O C K
    const Block &firstBlock = blockChain[0];
    auto timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(firstBlock.timestamp.time_since_epoch()).count();
    std::string hash = sha256(std::to_string(firstBlock.index) + firstBlock.data + std::to_string(timestampMS) +
                firstBlock.prevHash + std::to_string(firstBlock.difficulty) + std::to_string(firstBlock.nonce));

    if ((firstBlock.index != 0) || (firstBlock.prevHash != "0") || (firstBlock.hash != hash)) return false;

    // V A L I D A T E   T H E   R E S T   O F   B L O C K S
    for (unsigned int i = 1; i < size(); i++) {
        const Block &block = blockChain[i];
        const Block &prevBlock = blockChain[i - 1];
        timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(block.timestamp.time_since_epoch()).count();
        const auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(block.timestamp - prevBlock.timestamp);
        hash = sha256(std::to_string(block.index) + block.data + std::to_string(timestampMS) +
                    block.prevHash + std::to_string(block.difficulty) + std::to_string(block.nonce));

        if ((block.index != prevBlock.index + 1) || (block.prevHash != prevBlock.hash) || (timeDiff >= std::chrono::seconds(60)) || ((block.hash != hash))) return false;
    }

    return true;
}

unsigned int BlockChain::cumulativeDifficulty() const {
    unsigned int diffuculty = 0;
    for (const Block &block: blockChain) {
        diffuculty += std::pow(2, block.difficulty);
    }
    return diffuculty;
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


