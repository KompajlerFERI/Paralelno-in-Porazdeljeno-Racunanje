#include <fstream>
#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <thread>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <condition_variable>

#include "Block/Block.h"
#include "BlockChain/BlockChain.h"
#include "Rating/Rating.h"
#include "sha256/sha256.h"

// M P I
constexpr int MASTER_RANK = 0;
constexpr int DATA_TAG = 0;
constexpr int BLOCKCHAIN_TAG = 1;
constexpr unsigned int POOL_LIMIT = 50;
std::string COLOR_CODE;

// B L O C K   C H A I N
constexpr unsigned int INITIAL_DIFFICULTY = 3;
constexpr unsigned int BLOCK_GEN_INTERVAL = 2; // in seconds
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

std::shared_mutex blockchainSharedMutex;
std::mutex coutMutex, dataPoolMutex;
std::condition_variable dataPoolCV;

std::string getColorCode(const int rank) {
    switch (rank) {
        case 1: return RED;
        case 2: return GREEN;
        case 3: return YELLOW;
        case 4: return BLUE;
        case 5: return MAGENTA;
        case 6: return CYAN;
        default: return WHITE;
    }
}

void master(const int numberOfProcesses) {
    std::cout << "Hello from master (" << MASTER_RANK << ")" << std::endl;

    std::vector<std::string> pool;
    while(true){
        //TODO Spletna storitev

        Rating fakeData("3123123132131", 5);
        pool.push_back(fakeData.toJson().dump());

        if(pool.size() >= POOL_LIMIT){
            // S E N D   D A T A   T O   A L L   M I N E R S
            nlohmann::json json = pool;
            std::string jsonString = json.dump();
            std::vector<char> buffer(jsonString.begin(), jsonString.end());
            for(int i = 1; i < numberOfProcesses; i++){
                MPI_Send(buffer.data(), buffer.size(), MPI_CHAR, i, DATA_TAG, MPI_COMM_WORLD);
            }
            pool.clear();

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
}

unsigned int countLeadingCharacter(const std::string &text, const char character) {
    unsigned int counter = 0;
    while (text[counter] == character) ++counter;
    return counter;
}

void server(const int rank, const int numberOfProcesses, BlockChain &localBlockChain, std::queue<std::string> &dataPool) {
    MPI_Status status;
    int messageLength;

    // R E C E I V E   B L O C K C H A I N   F R O M   O T H E R   N O D E S
    while (true) {
        // Probe for an incoming message to get its size
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &messageLength);

        // Allocate buffer to receive the message
        std::vector<char> buffer(messageLength);
        MPI_Recv(buffer.data(), messageLength, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        /*std::cout << COLOR_CODE << "[" << rank <<  "]" << " JUST RECEIVED A MESSAGE FROM " << status.MPI_SOURCE << " " << status.MPI_TAG << RESET << std::endl;*/

        // Get Json string from buffer
        std::string jsonString(buffer.begin(), buffer.end());
        nlohmann::json json = nlohmann::json::parse(jsonString);
        // Deserialize Data
        if (status.MPI_TAG == DATA_TAG) {
            std::vector<std::string> receivedDataPool = json.get<std::vector<std::string>>();
            {
                std::unique_lock<std::mutex> lock(dataPoolMutex);
                for (const std::string& data: receivedDataPool) {
                    dataPool.push(data);
                }
                dataPoolCV.notify_one();
            }
        } // Deserialize JSON to BlockChain
        else {
            BlockChain receivedBlockChain = BlockChain::fromJson(json);

            std::shared_lock<std::shared_mutex> sharedLock(blockchainSharedMutex);
            if (receivedBlockChain.isValid() && (receivedBlockChain.cumulativeDifficulty() > localBlockChain.cumulativeDifficulty())) {
                sharedLock.unlock();

                std::unique_lock<std::shared_mutex> lock(blockchainSharedMutex);
                localBlockChain = receivedBlockChain;
                lock.unlock();
                {
                    std::unique_lock<std::mutex> coutLock(coutMutex);
                    std::cout << COLOR_CODE << "[" << rank << "] Local blockchain overridden by blockchain received from " << status.MPI_SOURCE << RESET << std::endl;
                }
            } else {
                std::unique_lock<std::mutex> coutLock(coutMutex);
                std::cout << COLOR_CODE << "[" << rank <<  "] Local blockchain is better" << RESET << std::endl;
            }
        }
    }
}

void client(const int rank, const int numberOfProcesses, const BlockChain &localBlockChain) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // S E N D   B L O C K C H A I N   T O   O T H E R   N O D E S
        std::shared_lock<std::shared_mutex> sharedLock(blockchainSharedMutex);
        if (!localBlockChain.empty()) {
            nlohmann::json json = localBlockChain.toJson();
            std::string jsonString = json.dump();
            std::vector<char> buffer(jsonString.begin(), jsonString.end());

            for (int i = 1; i < numberOfProcesses; ++i) {
                if (i != rank) {
                    MPI_Send(buffer.data(), buffer.size(), MPI_CHAR, i, BLOCKCHAIN_TAG, MPI_COMM_WORLD);
                }
            }
        }
    }
}

void miner(const int rank, const int numberOfProcesses) {
    std::cout << COLOR_CODE << "[" << rank << "] Hello from miner" << RESET << std::endl;
    BlockChain localBlockchain;
    std::queue<std::string> dataPool;
    unsigned int difficulty = INITIAL_DIFFICULTY;

    auto serverThread = std::thread(server, rank, numberOfProcesses, std::ref(localBlockchain), std::ref(dataPool));
    auto clientThread = std::thread(client, rank, numberOfProcesses, std::ref(localBlockchain));


    std::shared_lock<std::shared_mutex> sharedLock(blockchainSharedMutex);
    sharedLock.unlock();
    while(true) {
        sharedLock.lock();
        const unsigned int index = localBlockchain.empty() ? 0 : localBlockchain.getLastIndex() + 1;
        const std::string prevHash = localBlockchain.empty() ? "0" : localBlockchain[localBlockchain.size() - 1].hash;
        sharedLock.unlock();
        std::string data;
        const auto timestamp = std::chrono::system_clock::now();
        auto timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();
        Block block = Block(index, data, rank, timestamp, std::string(), prevHash, difficulty, 0);

        // P R O O F   O F   W O R K   -   M I N I N G
        bool hashFound = false;

        {
            std::unique_lock<std::mutex> dataPoolLock(dataPoolMutex);
            if(dataPool.empty()){
                dataPoolCV.wait(dataPoolLock, [&] { return !dataPool.empty();});
            }
            data = dataPool.front();
            block.data = data;
            dataPool.pop();
        }

        #pragma omp parallel
        {
            const int thread_id = omp_get_thread_num();
            const int num_threads = omp_get_num_threads();
            const t_ull step = ULLONG_MAX / num_threads;
            const t_ull start = thread_id * step;
            const t_ull end = (thread_id + 1) * step;

            std::string localHash;
            for (t_ull localNonce = start; localNonce < end && !hashFound; ++localNonce) {
                localHash = sha256(std::to_string(index) + data + std::to_string(timestampMS) + prevHash + std::to_string(difficulty) + std::to_string(localNonce));
                if (countLeadingCharacter(localHash, '0') >= difficulty) {
                #pragma omp critical
                    {
                        if (!hashFound) {
                            hashFound = true;
                            block.hash = localHash;
                            block.nonce = localNonce;
                        }
                    }
                }
            }
        }

        // V A L I D A T I O N
        timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(block.timestamp.time_since_epoch()).count();
        sharedLock.lock();
        if ((block.index == (localBlockchain.empty() ? 0 : localBlockchain.getLastIndex() + 1)) &&
            ((block.prevHash) == (localBlockchain.empty() ? "0" : localBlockchain.getLastHash())) &&
            (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - block.timestamp) < std::chrono::seconds(60)) &&
            (block.hash == sha256(std::to_string(block.index) + block.data + std::to_string(timestampMS) +
                block.prevHash + std::to_string(block.difficulty) + std::to_string(block.nonce)))
        ) {
            sharedLock.unlock();
            std::lock_guard<std::shared_mutex> lock(blockchainSharedMutex);
            localBlockchain.addBlock(block);
        } else {
            sharedLock.unlock();
        }

        /*if (rank == 1) {
            std::unique_lock<std::mutex> coutLock(coutMutex);
            sharedLock.lock();
            std::cout << COLOR_CODE << localBlockchain[localBlockchain.size() - 1].toString() << RESET << std::endl;
            sharedLock.unlock();
        }*/

        // A D J U S T   D I F F I C U L T Y
        sharedLock.lock();
        if(localBlockchain.size() % ADJUST_DIFFICULTY_INTERVAL == 0) {
            const Block prevAdjustmentBlock = localBlockchain[localBlockchain.size() - BLOCK_GEN_INTERVAL];
            const Block lastBlock = localBlockchain[localBlockchain.size() - 1];
            sharedLock.unlock();
            constexpr unsigned int timeExpected = BLOCK_GEN_INTERVAL * ADJUST_DIFFICULTY_INTERVAL;
            const unsigned int timeTaken = std::chrono::duration_cast<std::chrono::seconds>(lastBlock.timestamp - prevAdjustmentBlock.timestamp).count();

            if (timeTaken < (timeExpected / 2)) {
                difficulty = prevAdjustmentBlock.difficulty + 1;
                {
                    std::unique_lock<std::mutex> coutLock(coutMutex);
                    std::cout << COLOR_CODE << "[" << rank << "] Difficulty increased to " << difficulty << RESET << std::endl;
                }
            }
            else if(timeTaken > (timeExpected * 2)) {
                difficulty = prevAdjustmentBlock.difficulty - 1;
                {
                    std::unique_lock<std::mutex> coutLock(coutMutex);
                    std::cout << COLOR_CODE << "[" << rank << "] Difficulty decreased to " << difficulty << RESET << std::endl;
                }
            } else {
                std::unique_lock<std::mutex> coutLock(coutMutex);
                std::cout << COLOR_CODE << "[" << rank << "] Difficulty did not change" << std::endl;
            }

            /*{
                std::unique_lock<std::mutex> coutLock(coutMutex);
                sharedLock.lock();
                std::cout << COLOR_CODE << "[" << rank <<  "] Blockchain cumulative difficulty: " << localBlockchain.cumulativeDifficulty() << RESET << std::endl;
                sharedLock.unlock();
            }*/

            sharedLock.lock();
            if (localBlockchain.isValid()) {
                sharedLock.unlock();
                {
                    std::unique_lock<std::mutex> coutLock(coutMutex);
                    std::cout << COLOR_CODE << "[" << rank << "] Blockchain still valid" << RESET << std::endl;
                }
            } else {
                sharedLock.unlock();
                {
                    std::unique_lock<std::mutex> coutLock(coutMutex);
                    std::cout << COLOR_CODE << "[" << rank << "] Blockchain no longer valid" << RESET << std::endl;
                }
            }
        } else {
            sharedLock.unlock();
        }
    }
    std::cout << COLOR_CODE << "[" << rank << "] 14" << RESET << std::endl;
    serverThread.join();
    clientThread.join();
}

int main(const int argc, char *argv[])
{
    // M P I
    int rank;
    int numberOfProcesses;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    COLOR_CODE = getColorCode(rank);

    if(rank == MASTER_RANK) {
        master(numberOfProcesses);
    } else {
        miner(rank, numberOfProcesses);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}
