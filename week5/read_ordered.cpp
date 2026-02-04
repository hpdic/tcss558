#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdio>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char* filename = "read_eof.bin";
    const int TOTAL_FILE_SIZE = 100; // The file only has 100 bytes
    const int READ_SIZE = 60;        // Each rank wants 60 bytes

    // ======================================================
    // Step 1: Preparation (Rank 0 creates a small file)
    // ======================================================
    if (rank == 0) {
        MPI_File fh_write;
        MPI_File_open(MPI_COMM_SELF, filename, 
                      MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh_write);
        
        std::vector<char> dummy_data(TOTAL_FILE_SIZE, 'X');
        MPI_File_write(fh_write, dummy_data.data(), TOTAL_FILE_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
        MPI_File_close(&fh_write);
        printf("[Setup] Created file with %d bytes.\n", TOTAL_FILE_SIZE);
    }
    // Ensure file exists before everyone opens it
    MPI_Barrier(MPI_COMM_WORLD); 

    // ======================================================
    // Step 2: The Trap Scenario (Read Ordered)
    // ======================================================
    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

    std::vector<char> buffer(READ_SIZE, '.');
    MPI_Status status;

    // All ranks request 60 bytes.
    // If 2 ranks run this: 2 * 60 = 120 bytes needed.
    // But file only has 100 bytes!
    int err = MPI_File_read_ordered(fh, buffer.data(), READ_SIZE, MPI_CHAR, &status);

    // ======================================================
    // Step 3: The Solution (Check MPI_Get_count)
    // ======================================================
    int count_got;
    MPI_Get_count(&status, MPI_CHAR, &count_got);

    // Even if we hit EOF, err is usually MPI_SUCCESS! 
    // We MUST check count_got to find the truth.
    if (count_got < READ_SIZE) {
        printf("[Rank %d] WARNING: Hit EOF! Wanted %d, but got %d bytes.\n", 
               rank, READ_SIZE, count_got);
    } else {
        printf("[Rank %d] Success: Read full %d bytes.\n", rank, count_got);
    }
    std::cout << "Rank " << rank << " buffer: '" << std::string(buffer.begin(), buffer.end()) << "'\n";
    std::cout.flush();

    MPI_File_close(&fh);
    MPI_Finalize();
    return 0;
}

/**
mpic++ -o read_ordered.bin read_ordered.cpp
mpirun -np 2 ./read_ordered.bin
 */