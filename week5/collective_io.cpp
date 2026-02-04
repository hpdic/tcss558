#include <mpi.h>
#include <iostream>
#include <vector>
#include <numeric> // for std::iota

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = 10; // Number of integers per rank
    const char* filename = "test_collective.bin";

    // 1. Prepare Data
    // Rank 0: [0, 1, ..., 9], Rank 1: [10, 11, ..., 19]
    std::vector<int> write_buf(N);
    std::iota(write_buf.begin(), write_buf.end(), rank * N);

    MPI_File fh;
    MPI_Status status;

    // Open file for Read/Write. Create if doesn't exist.
    MPI_File_open(MPI_COMM_WORLD, filename, 
                  MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

    // =============================================================
    // PART 1: MPI_File_write_at_all
    // =============================================================
    
    // Calculate explicit offset (GPS coordinates)
    // Rank 0 starts at 0 bytes, Rank 1 starts at 40 bytes
    MPI_Offset offset = rank * N * sizeof(int);

    // Collective Write: Everyone writes together for optimization
    MPI_File_write_at_all(fh, offset, write_buf.data(), N, MPI_INT, &status);

    // Ensure data hits the disk before reading back
    MPI_File_sync(fh); 
    
    // =============================================================
    // PART 2: MPI_File_read_at_all
    // =============================================================

    std::vector<int> read_buf(N, 0); // Initialize with zeros

    // Collective Read: Everyone reads their chunk together
    MPI_File_read_at_all(fh, offset, read_buf.data(), N, MPI_INT, &status);

    // =============================================================
    // Verification
    // =============================================================
    
    // Print first and last element to verify
    printf("[Rank %d] Wrote: %d..%d | Read: %d..%d\n", 
           rank, write_buf.front(), write_buf.back(), 
           read_buf.front(), read_buf.back());

    MPI_File_close(&fh);
    MPI_Finalize();
    return 0;
}

/**
mpic++ -o collective_io.bin collective_io.cpp
mpirun -np 2 ./collective_io.bin
 */