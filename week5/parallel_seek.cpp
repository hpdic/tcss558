#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const std::string filename = "output_parallel_seek.bin";

    // 1. Simulate variable data size per rank
    srand(time(NULL) + rank);
    long long local_bytes = (rand() % 100) + 1; // 1 to 100 bytes
    std::vector<char> buffer(local_bytes, 'A' + rank);

    long long my_offset = 0;
    long long incoming_offset = 0;
    MPI_Request req_send, req_recv;
    MPI_Status status;

    // 2. Calculate Offset using Point-to-Point communication
    if (rank == 0) {
        // Rank 0 always starts at 0
        my_offset = 0;
        
        // Pass the new total to Rank 1
        long long next_offset = my_offset + local_bytes;
        if (size > 1) {
            MPI_Isend(&next_offset, 1, MPI_LONG_LONG, rank + 1, 0, MPI_COMM_WORLD, &req_send);
            // Must wait or verify completion (though Isend returns immediately)
            MPI_Wait(&req_send, MPI_STATUS_IGNORE); 
        }
    } 
    else {
        // Receive offset from previous rank
        MPI_Irecv(&incoming_offset, 1, MPI_LONG_LONG, rank - 1, 0, MPI_COMM_WORLD, &req_recv);
        MPI_Wait(&req_recv, &status);
        
        // My offset is exactly what I received
        my_offset = incoming_offset;

        // If I am not the last one, pass the accumulated sum to the next rank
        if (rank < size - 1) {
            long long next_offset = my_offset + local_bytes;
            MPI_Isend(&next_offset, 1, MPI_LONG_LONG, rank + 1, 0, MPI_COMM_WORLD, &req_send);
            MPI_Wait(&req_send, MPI_STATUS_IGNORE);
        }
    }

    // 3. Write to file
    // Rank 0 creates file
    if (rank == 0) {
        std::ofstream ofs(filename, std::ios::binary);
        ofs.close();
    }
    MPI_Barrier(MPI_COMM_WORLD);

    std::fstream fs(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (fs.is_open()) {
        fs.seekp(my_offset);
        fs.write(buffer.data(), local_bytes);
        fs.close();
        
        // Verify output
        std::cout << "Rank " << rank << " offset: " << my_offset 
                  << " size: " << local_bytes << std::endl;
    }

    MPI_Finalize();
    return 0;
}

/**
mpic++ parallel_seek.cpp -o parallel_seek.bin
mpirun -np 2 ./parallel_seek.bin
 */