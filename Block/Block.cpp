#include "Block.h"

#include <iomanip>

#include "../BlockChain/BlockChain.h"

Block::Block(): index(0), data(""), miner(0), timestamp(timestamp), hash(hash), prevHash(prevHash), difficulty(difficulty), nonce(nonce)  {}

Block::Block(
    const unsigned int index,
    const std::string &data,
    const unsigned int miner,
    const std::chrono::system_clock::time_point &timestamp,
    const std::string &hash,
    const std::string &prevHash,
    const unsigned int difficulty,
    const t_ull nonce
): index(index), data(data), miner(miner), timestamp(timestamp), hash(hash), prevHash(prevHash), difficulty(difficulty), nonce(nonce) {}

std::string Block::toString() const {
    std::ostringstream oss;
    const std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    oss << "---------[ B L O C K ]---------\n"
        << "index: " << index << '\n'
        << "data: " << data << '\n'
        << "miner: " << miner << '\n'
        << "timestamp: " << std::put_time(std::localtime(&time), "%d-%m-%Y %H:%M:%S") << '\n'
        << "hash: " << hash << '\n'
        << "prevHash: " << prevHash << '\n'
        << "difficulty: " << difficulty << '\n'
        << "nonce: " << nonce << '\n';
    return oss.str();
}
