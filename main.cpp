#include <iostream>

#include <mpi.h>

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
    printf("Hello world from processor %s, rank %d out of %d processors\n", processor_name);

    // Finalize the MPI environment.
    MPI_Finalize();
}
