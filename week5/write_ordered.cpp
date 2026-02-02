#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char* filename = "output_write_ordered.bin";

    srand(time(NULL) + rank);
    int local_count = (rand() % 100) + 1;
    std::vector<char> buffer(local_count, 'A' + rank);

    MPI_File fh;
    // Open with CREATE | WRONLY
    int err = MPI_File_open(MPI_COMM_WORLD, filename, 
                            MPI_MODE_CREATE | MPI_MODE_WRONLY, 
                            MPI_INFO_NULL, &fh);

    if (err != MPI_SUCCESS) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // This truncates the file to 0 bytes safely.
    MPI_File_set_size(fh, 0);
    
    MPI_Status status;
    // Everyone calls this together
    MPI_File_write_ordered(fh, buffer.data(), local_count, MPI_CHAR, &status);
    printf("Rank %d wrote %d bytes.\n", rank, local_count);

    MPI_File_close(&fh);

    MPI_Finalize();
    return 0;
}

/**
mpic++ -o write_ordered.bin write_ordered.cpp
mpirun -np 2 ./write_ordered.bin
 */