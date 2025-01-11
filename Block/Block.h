#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include <chrono>

using t_ull = unsigned long long;

class Block {
public:
    unsigned int index;
    std::string data;
    unsigned int miner;
    std::chrono::system_clock::time_point timestamp;
    std::string hash;
    std::string prevHash;

    unsigned int difficulty;
    t_ull nonce;

    Block();
    Block(const unsigned int index, const std::string &data, const unsigned int miner, const std::chrono::system_clock::time_point &timestamp, const std::string &hash, const std::string &prevHash, const unsigned int difficulty, const t_ull nonce);

    std::string toString() const;
};



#endif //BLOCK_H
