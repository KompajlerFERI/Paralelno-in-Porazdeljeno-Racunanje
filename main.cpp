#include <algorithm>
#include <iostream>

#include <mpi.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

#include "Block/Block.h"
#include "BlockChain/BlockChain.h"

// M P I
constexpr int MASTER_RANK = 0;

// B L O C K   C H A I N
constexpr unsigned int INITIAL_DIFFICULTY = 3;
constexpr unsigned int BLOCK_GEN_INTERVAL = 2; // in secondss
constexpr unsigned int ADJUST_DIFFICULTY_INTERVAL = 5; // every 5 blocks generated

// A N S I   C O L O R   E S C A P E   C O D E S
const std::string BLACK = "\033[30m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";
const std::string RESET = "\033[0m";


std::string sha256(const std::string& str) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context != nullptr) {
        if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr)) {
            if (EVP_DigestUpdate(context, str.c_str(), str.size())) {
                if (EVP_DigestFinal_ex(context, hash, &lengthOfHash)) {
                    std::stringstream ss;
                    for (unsigned int i = 0; i < lengthOfHash; ++i) {
                        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
                    }
                    EVP_MD_CTX_free(context);
                    return ss.str();
                }
            }
        }
        EVP_MD_CTX_free(context);
    }
    return "";
}

void master(const int numberOfProcesses) {
    std::cout << "Hello from master (" << MASTER_RANK << ")" << std::endl;
}

unsigned int countLeadingCharacter(const std::string &text, const char character) {
    unsigned int counter = 0;
    while (text[counter] == character) ++counter;
    return counter;
}

void miner(const int rank) {
    std::cout << "Hello from miner (" << rank << ")" << std::endl;

    BlockChain localBlockchain;
    unsigned int difficulty = INITIAL_DIFFICULTY;

    while(true) {
        const unsigned int index = localBlockchain.empty() ? 0 : localBlockchain.getLastIndex() + 1;
        const std::string data = "To je blok [" + std::to_string(index) + "]";
        const auto timestamp = std::chrono::system_clock::now();
        auto timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();
        std::string hash;
        const std::string prevHash = localBlockchain.empty() ? "0" : localBlockchain[localBlockchain.size() - 1].hash;
        Block block = Block(index, data, rank, timestamp, hash, prevHash, difficulty, 0);

        // P R O O F   O F   W O R K   -   M I N I N G
        t_ull nonce = 0;
        while (true) {
            hash = sha256(std::to_string(index) + data + std::to_string(timestampMS) + prevHash + std::to_string(difficulty) + std::to_string(nonce));
            if (countLeadingCharacter(hash, '0') >= difficulty) {
                block.hash = hash;
                block.nonce = nonce;
                break;
            }
            nonce++;
        }

        // V A L I D A T I O N
        timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(block.timestamp.time_since_epoch()).count();
        if ((block.index == (localBlockchain.empty() ? 0 : localBlockchain.getLastIndex() + 1)) &&
            ((block.prevHash) == (localBlockchain.empty() ? "0" : localBlockchain.getLastHash())) &&
            (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - block.timestamp) < std::chrono::seconds(60)) &&
            (block.hash == sha256(std::to_string(block.index) + block.data + std::to_string(timestampMS) +
                block.prevHash + std::to_string(block.difficulty) + std::to_string(block.nonce)))
        ) {
            localBlockchain.addBlock(block);
        }

        std::cout << localBlockchain[localBlockchain.size() - 1].toString() << std::endl;

        // A D J U S T   D I F F I C U L T Y
        if(localBlockchain.size() % ADJUST_DIFFICULTY_INTERVAL == 0) {
            const Block& prevAdjustmentBlock = localBlockchain[localBlockchain.size() - BLOCK_GEN_INTERVAL];
            const Block& lastBlock = localBlockchain[localBlockchain.size() - 1];
            constexpr unsigned int timeExpected = BLOCK_GEN_INTERVAL * ADJUST_DIFFICULTY_INTERVAL;
            const unsigned int timeTaken = std::chrono::duration_cast<std::chrono::seconds>(lastBlock.timestamp - prevAdjustmentBlock.timestamp).count();

            if (timeTaken < (timeExpected / 2)) {
                difficulty = prevAdjustmentBlock.difficulty + 1;
                std::cout << '\n' << MAGENTA << "Difficulty increased to " << difficulty << RESET << "\n\n" <<std::endl;
            }
            else if(timeTaken > (timeExpected * 2)) {
                difficulty = prevAdjustmentBlock.difficulty - 1;
                std::cout << '\n' << MAGENTA << "Difficulty decreased to " << difficulty << "\n\n" <<std::endl;
            } else std::cout << '\n' << MAGENTA << "Difficulty did not change" << RESET << "\n\n" <<std::endl;
        }
    }
}

int main(const int argc, char *argv[])
{
    // M P I
    int rank;
    int numberOfProcesses;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == MASTER_RANK) {
        master(numberOfProcesses);
    } else {
        miner(rank);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}
