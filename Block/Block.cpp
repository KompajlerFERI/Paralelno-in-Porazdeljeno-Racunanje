#include "Block.h"

#include <iomanip>
#include <iostream>

#include "../BlockChain/BlockChain.h"

Block::Block() : index(0), data(""), miner(0), timestamp(std::chrono::system_clock::time_point()), hash(""), prevHash(""), difficulty(0), nonce(0) {}
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

nlohmann::json Block::toJson() const {
    nlohmann::json j;
    j["index"] = index;
    j["data"] = data;
    j["miner"] = miner;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();
    j["hash"] = hash;
    j["prevHash"] = prevHash;
    j["difficulty"] = difficulty;
    j["nonce"] = nonce;
    return j;
}

Block Block::fromJson(const nlohmann::json &j) {
    Block block;
    try {
        block.index = j.at("index").get<unsigned int>();
        block.data = j.at("data").get<std::string>();
        block.miner = j.at("miner").get<unsigned int>();
        block.timestamp = std::chrono::system_clock::time_point(std::chrono::milliseconds(j.at("timestamp").get<long long>()));
        block.hash = j.at("hash").get<std::string>();
        block.prevHash = j.at("prevHash").get<std::string>();
        block.difficulty = j.at("difficulty").get<unsigned int>();
        block.nonce = j.at("nonce").get<t_ull>();
    } catch (const std::exception &e) {
        std::cerr << "Error parsing Block JSON: " << e.what() << std::endl;
    }
    return block;
}

