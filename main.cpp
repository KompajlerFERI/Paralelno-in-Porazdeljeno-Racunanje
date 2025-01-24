#include <fstream>
#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <condition_variable>

#include "Block/Block.h"
#include "BlockChain/BlockChain.h"
#include "Rating/Rating.h"
#include "sha256/sha256.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

// M P I
constexpr int MASTER_RANK = 0;
constexpr int DATA_TAG = 0;
constexpr int BLOCK_FOUND_TAG = 1;
constexpr int BLOCK_TAG = 2;
constexpr unsigned int POOL_LIMIT = 10;
std::string COLOR_CODE;

// B L O C K   C H A I N
constexpr unsigned int INITIAL_DIFFICULTY = 5;
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

    std::string testMessage = "test";
    std::vector<char> testBuffer(testMessage.begin(), testMessage.end());

    for (int j = 0; j < 20; j++) {
        for (int i = 1; i < numberOfProcesses; i++) {
            MPI_Send(testBuffer.data(), testBuffer.size(), MPI_CHAR, i, DATA_TAG, MPI_COMM_WORLD);
        }
    }

    std::cout << "Data has been sent to miners " << std::endl;

    /*// Create a web server
    httplib::Server svr;

    // Define a route to send data to miners immediately
    svr.Post("/add_data", [numberOfProcesses](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received POST request: " << req.body << std::endl;
        try {
            const auto json = nlohmann::json::parse(req.body);
            const Rating rating = Rating::fromJson(json);

            std::string jsonString = rating.toJson().dump();
            std::vector<char> buffer(jsonString.begin(), jsonString.end());

            // Send data to all miners immediately
            for (int i = 1; i < numberOfProcesses; i++) {
                MPI_Send(buffer.data(), buffer.size(), MPI_CHAR, i, DATA_TAG, MPI_COMM_WORLD);
                std::cout << "Data sent to miner " << i << std::endl;
            }

            res.set_content("Data added and sent to miners", "text/plain");
        } catch (const std::exception& e) {
            std::cerr << "Error processing POST request: " << e.what() << std::endl;
            res.status = 400;
            res.set_content("Invalid data", "text/plain");
        }
    });

    // Start the server in a separate thread
    std::thread serverThread([&svr]() {
        std::cout << "Starting server on port 8080" << std::endl;
        if (!svr.listen("0.0.0.0", 8080)) {
            std::cerr << "Error starting server" << std::endl;
        }
    });

    serverThread.join();*/
}

unsigned int countLeadingCharacter(const std::string &text, const char character) {
    unsigned int counter = 0;
    while (text[counter] == character) ++counter;
    return counter;
}


void miner(const int rank, const int numberOfProcesses) {
    std::cout << COLOR_CODE << "[" << rank << "] Hello from miner" << RESET << std::endl;
    BlockChain localBlockchain;
    unsigned int difficulty = INITIAL_DIFFICULTY;
    MPI_Status status;
    int messageLength;

    auto startTime = std::chrono::high_resolution_clock::now();
    while(localBlockchain.size() < 20) {
        MPI_Probe(MASTER_RANK, DATA_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &messageLength);

        std::vector<char> buffer(messageLength);
        MPI_Recv(buffer.data(), messageLength, MPI_CHAR, MASTER_RANK, DATA_TAG, MPI_COMM_WORLD, &status);

        std::string data(buffer.begin(), buffer.end());

        const unsigned int index = localBlockchain.empty() ? 0 : localBlockchain.getLastIndex() + 1;
        const std::string prevHash = localBlockchain.empty() ? "0" : localBlockchain[localBlockchain.size() - 1].hash;
        const auto timestamp = std::chrono::system_clock::now();
        auto timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();
        Block block = Block(index, data, rank, timestamp, std::string(), prevHash, difficulty, 0);

        // P R O O F   O F   W O R K   -   M I N I N G
        bool globalHashFound = false;
        bool localHashFound = false;
        #pragma omp parallel
        {
            const int thread_id = omp_get_thread_num();
            const int num_threads = omp_get_num_threads();
            const t_ull step = ULLONG_MAX / num_threads;
            const t_ull start = thread_id * step;
            const t_ull end = (thread_id + 1) * step;

            std::string localHash;
            for (t_ull localNonce = start; localNonce < end && !localHashFound && !globalHashFound; ++localNonce) {
                localHash = sha256(std::to_string(index) + data + std::to_string(timestampMS) + prevHash + std::to_string(difficulty) + std::to_string(localNonce));
                if (countLeadingCharacter(localHash, '0') >= difficulty) {
                    #pragma omp critical
                    {
                        if (!localHashFound) {
                            localHashFound = true;
                            globalHashFound = true;
                            block.hash = localHash;
                            block.nonce = localNonce;
                        }
                    }
                }
                // C H E C K   I F   O T H E R   M I N E R   H A S   A L R E A D Y   M I N E D   T H E   B L O C K
                #pragma omp critical
                {
                    int flag;
                    MPI_Iprobe(MPI_ANY_SOURCE, BLOCK_FOUND_TAG, MPI_COMM_WORLD, &flag, &status);
                    if (flag) {
                        MPI_Recv(&globalHashFound, 1, MPI_C_BOOL, MPI_ANY_SOURCE, BLOCK_FOUND_TAG, MPI_COMM_WORLD, &status);
                    }
                }
            }
        }

        // N O T I F Y   O T H E R   M I N E R S
        if (localHashFound) {
            for (int i = 1; i < numberOfProcesses; i++) {
                if (i != rank) {
                    MPI_Send(&globalHashFound, 1, MPI_C_BOOL, i, BLOCK_FOUND_TAG, MPI_COMM_WORLD);
                }
            }
        }

        // S E N D   L O C A L   B L O C K   T O   O T H E R   M I N E R S
        std::string blockJsonString = block.toJson().dump();
        std::vector<char> blockBuffer(blockJsonString.begin(), blockJsonString.end());
        for (int i = 1; i < numberOfProcesses; i++) {
            if (i != rank) {
                MPI_Send(blockBuffer.data(), blockBuffer.size(), MPI_CHAR, i, BLOCK_TAG, MPI_COMM_WORLD);
            }
        }

        // R E C E I V E   B L O C K S   F R O M   O T H E R   M I N E R S
        std::vector<Block> receivedBlocks(numberOfProcesses - 1);
        receivedBlocks[rank - 1] = block;
        bool allBlocksReceived = false;
        if (numberOfProcesses != 2) {
            while (!allBlocksReceived) {
                MPI_Probe(MPI_ANY_SOURCE, BLOCK_TAG, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_CHAR, &messageLength);

                std::vector<char> recvBuffer(messageLength);
                MPI_Recv(recvBuffer.data(), messageLength, MPI_CHAR, MPI_ANY_SOURCE, BLOCK_TAG, MPI_COMM_WORLD, &status);

                std::string recvData(recvBuffer.begin(), recvBuffer.end());
                Block receivedBlock = Block::fromJson(nlohmann::json::parse(recvData));
                receivedBlocks[status.MPI_SOURCE - 1] = receivedBlock;

                for (int i = 0; i < numberOfProcesses - 1; i++) {
                    if (receivedBlocks[i].miner != 0) {
                        allBlocksReceived = true;
                    } else {
                        allBlocksReceived = false;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < numberOfProcesses - 1; i++) {
            Block currentBlock = receivedBlocks[i];
            // V A L I D A T I O N
            timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(currentBlock.timestamp.time_since_epoch()).count();
            if ((currentBlock.index == (localBlockchain.empty() ? 0 : localBlockchain.getLastIndex() + 1)) &&
                ((currentBlock.prevHash) == (localBlockchain.empty() ? "0" : localBlockchain.getLastHash())) &&
                (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - currentBlock.timestamp) < std::chrono::seconds(60)) &&
                (currentBlock.hash == sha256(std::to_string(currentBlock.index) + currentBlock.data + std::to_string(timestampMS) +
                    currentBlock.prevHash + std::to_string(currentBlock.difficulty) + std::to_string(currentBlock.nonce)))
            ) {
                std::lock_guard<std::shared_mutex> lock(blockchainSharedMutex);
                localBlockchain.addBlock(currentBlock);
                break;
            }
        }

        if (rank == 1) {
            std::cout << getColorCode(localBlockchain[localBlockchain.getLastIndex()].miner) << localBlockchain[localBlockchain.getLastIndex()].toString() << RESET << std::endl;
        }

        int flag;
        MPI_Status status;

        do {
            MPI_Iprobe(MPI_ANY_SOURCE, BLOCK_FOUND_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                bool dummy;
                MPI_Recv(&dummy, 1, MPI_C_BOOL, MPI_ANY_SOURCE, BLOCK_FOUND_TAG, MPI_COMM_WORLD, &status);
            }
        } while (flag);

        // A D J U S T   D I F F I C U L T Y
        /*if(localBlockchain.size() % ADJUST_DIFFICULTY_INTERVAL == 0) {
            const Block prevAdjustmentBlock = localBlockchain[localBlockchain.size() - BLOCK_GEN_INTERVAL];
            const Block lastBlock = localBlockchain[localBlockchain.size() - 1];
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

            {
                std::unique_lock<std::mutex> coutLock(coutMutex);
                std::cout << COLOR_CODE << "[" << rank <<  "] Blockchain cumulative difficulty: " << localBlockchain.cumulativeDifficulty() << RESET << std::endl;
            }

            if (localBlockchain.isValid()) {
                {
                    std::unique_lock<std::mutex> coutLock(coutMutex);
                    std::cout << COLOR_CODE << "[" << rank << "] Blockchain still valid" << RESET << std::endl;
                }
            } else {
                {
                    std::unique_lock<std::mutex> coutLock(coutMutex);
                    std::cout << COLOR_CODE << "[" << rank << "] Blockchain no longer valid" << RESET << std::endl;
                }
            }
        }*/
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();
    if (rank == 1) {
        std::cout << COLOR_CODE << "[" << rank << "] Mining process took " << duration << " seconds"  << RESET << std::endl;
        if (localBlockchain.isValid()) {
            std::cout << GREEN << "[" << rank << "] Blockchain is valid "  << RESET << std::endl;
        } else {
            std::cout << RED << "[" << rank << "] Blockchain isn't valid "  << RESET << std::endl;
        }
    }
}

int main(const int argc, char *argv[]) {
    // M P I
    int rank;
    int numberOfProcesses;
    int provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        std::cerr << "MPI does not support multiple threads" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    COLOR_CODE = getColorCode(rank);

    if (rank == MASTER_RANK) {
        master(numberOfProcesses);
    } else {
        miner(rank, numberOfProcesses);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}
