#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>

#define TAG_WORK 1
#define TAG_STOP 0

const int MAX_NUM = 10000000; 
const int CHUNK_SIZE = 50000; 

bool is_prime(int n) {
    if (n < 2) return false;
    int limit = std::sqrt(n);
    for (int i = 2; i <= limit; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

// -------------------------------------------------------------------
// ROLE: COORDINATOR (Rank 0)
// -------------------------------------------------------------------
void run_coordinator(int size) {
    int tasks_assigned = 0;
    int current_start = 2; 
    int workers_active = size - 1; 
    long total_primes_found = 0;

    MPI_Status status;
    int received_count = 0;

    // Start Timer
    double start_time = MPI_Wtime();

    std::cout << "[Coordinator] Starting job distribution..." << std::endl;

    while (workers_active > 0) {
        MPI_Recv(&received_count, 1, MPI_INT, MPI_ANY_SOURCE, 
                 MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        total_primes_found += received_count;
        int worker_rank = status.MPI_SOURCE;

        if (current_start <= MAX_NUM) {
            int range[2];
            range[0] = current_start;
            range[1] = std::min(current_start + CHUNK_SIZE - 1, MAX_NUM);

            MPI_Send(range, 2, MPI_INT, worker_rank, TAG_WORK, MPI_COMM_WORLD);

            current_start += CHUNK_SIZE;
            tasks_assigned++;
        } else {
            MPI_Send(nullptr, 0, MPI_INT, worker_rank, TAG_STOP, MPI_COMM_WORLD);
            workers_active--;
        }
    }

    // Stop Timer
    double end_time = MPI_Wtime();
    double total_duration = end_time - start_time;

    std::cout << "[Coordinator] All workers finished." << std::endl;
    std::cout << "[Coordinator] Total Primes Found: " << total_primes_found << std::endl;
    
    // Output formatted time
    std::cout << "[Coordinator] Wall Clock Time: " 
              << std::fixed << std::setprecision(4) << total_duration << " seconds" << std::endl;
    std::cout << "[Coordinator] Total Chunks Processed: " << tasks_assigned << std::endl;
}

// -------------------------------------------------------------------
// ROLE: WORKER (Rank > 0)
// -------------------------------------------------------------------
void run_worker(int rank) {
    int primes_found = 0;
    int range[2];
    MPI_Status status;
    int chunks_processed = 0; // Stats counter

    // Send initial ready signal
    MPI_Send(&primes_found, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    while (true) {
        MPI_Recv(range, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_STOP) {
            break; 
        }

        // Process the chunk
        primes_found = 0;
        for (int i = range[0]; i <= range[1]; i++) {
            if (is_prime(i)) {
                primes_found++;
            }
        }
        chunks_processed++;

        MPI_Send(&primes_found, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    printf("[Worker %d] Finished. Processed %d chunks.\n", rank, chunks_processed);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) std::cerr << "Error: Requires at least 2 ranks." << std::endl;
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        run_coordinator(size);
    } else {
        run_worker(rank);
    }

    MPI_Finalize();
    return 0;
}

/**
mpic++ prime_dynamic.cpp -o prime_dynamic.bin
mpirun -np 3 ./prime_dynamic.bin
 */