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
    bool isValid() const;
    unsigned int cumulativeDifficulty() const;

    void addBlock(const Block &block);

    std::string toString() const;
    nlohmann::json toJson() const;

    Block& operator[](const unsigned int index);
    const Block& operator[](const unsigned int index) const;

    static BlockChain fromJson(const nlohmann::json &j);
};



#endif //BLOCKCHAIN_H
