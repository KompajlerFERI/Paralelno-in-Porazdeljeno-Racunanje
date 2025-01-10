#include <iostream>

#include <mpi.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>


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

int main(const int argc, char *argv[])
{
    // M P I
    int rank;
    int numberOfProcesses;



    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    /*printf("Hello world from processor %s, rank %d out of %d processors\n", processor_name);*/

    // Print hash of "hello world"
    if(rank == 0) {
        std::string temp = "Hello world";
        std::cout << "\"Hello world\" hashed: " << sha256(temp) << std::endl;
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}
