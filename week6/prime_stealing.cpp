#include <mpi.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

const int GLOBAL_MAX = 10000000; 
const int SPLIT_THRESHOLD = 1000; 
const int POLL_FREQ = 100;

#define TAG_STEAL_REQ 10 
#define TAG_WORK_DATA 20 
#define TAG_STOP      30 

bool is_prime(int n) {
    if (n < 2) return false;
    int limit = std::sqrt(n);
    for (int i = 2; i <= limit; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

void run_p2p_node(int rank, int size) {
    int target_rank = (rank == 0) ? 1 : 0;
    
    // Track if partner is still alive. 
    // If I rejected him once, I know he exited, so I shouldn't steal from him later.
    bool partner_active = true; 

    long current_idx, end_idx;
    long half_point = GLOBAL_MAX / 2;

    // Static Split
    if (rank == 0) { current_idx = 2; end_idx = half_point; } 
    else { current_idx = half_point + 1; end_idx = GLOBAL_MAX; }

    long total_primes = 0;
    long tasks_stolen = 0;
    long tasks_shared = 0;
    int  dummy_buf = 0; 

    double t_start = MPI_Wtime();

    while (true) {
        // ==========================================
        // PHASE 1: WORKING STATE
        // ==========================================
        while (current_idx <= end_idx) {
            if (is_prime(current_idx)) total_primes++;

            // Polling Logic
            if (current_idx % POLL_FREQ == 0) {
                int flag = 0;
                MPI_Status probe_status;
                MPI_Iprobe(target_rank, TAG_STEAL_REQ, MPI_COMM_WORLD, &flag, &probe_status);

                if (flag) {
                    // Recv the request
                    MPI_Recv(&dummy_buf, 1, MPI_INT, target_rank, TAG_STEAL_REQ, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    long remaining = end_idx - current_idx;
                    if (remaining > SPLIT_THRESHOLD) {
                        // Share work
                        long share = remaining / 2;
                        long split_start = end_idx - share + 1;
                        long split_end = end_idx;
                        end_idx = end_idx - share;

                        long work_packet[2] = {split_start, split_end};
                        MPI_Send(work_packet, 2, MPI_LONG, target_rank, TAG_WORK_DATA, MPI_COMM_WORLD);
                        tasks_shared++;
                    } else {
                        // Reject and mark partner as dead
                        MPI_Send(&dummy_buf, 0, MPI_INT, target_rank, TAG_STOP, MPI_COMM_WORLD);
                        partner_active = false; 
                    }
                }
            }
            current_idx++;
        }

        // ==========================================
        // PHASE 2: THIEF STATE (Non-blocking Fix)
        // ==========================================
        
        // If partner is already dead, I can't steal. Exit.
        if (!partner_active) break;

        // Send Steal Request
        MPI_Send(&dummy_buf, 0, MPI_INT, target_rank, TAG_STEAL_REQ, MPI_COMM_WORLD);

        // Wait for response BUT handle incoming collisions
        bool received_response = false;
        while (!received_response) {
            int flag = 0;
            MPI_Status status;
            
            // Check for ANY message (Reply OR New Request)
            MPI_Iprobe(target_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (flag) {
                if (status.MPI_TAG == TAG_WORK_DATA) {
                    // Success: Got work
                    long work_packet[2];
                    MPI_Recv(work_packet, 2, MPI_LONG, target_rank, TAG_WORK_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    current_idx = work_packet[0];
                    end_idx = work_packet[1];
                    tasks_stolen++;
                    received_response = true; // Go back to Phase 1
                } 
                else if (status.MPI_TAG == TAG_STOP) {
                    // Failure: Partner rejected me
                    MPI_Recv(&dummy_buf, 0, MPI_INT, target_rank, TAG_STOP, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    partner_active = false;
                    received_response = true; // Will break outer loop
                }
                else if (status.MPI_TAG == TAG_STEAL_REQ) {
                    // COLLISION: He is asking me while I am asking him.
                    // This means WE ARE BOTH EMPTY.
                    MPI_Recv(&dummy_buf, 0, MPI_INT, target_rank, TAG_STEAL_REQ, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    
                    // Send STOP to him so he can exit cleanly too
                    MPI_Send(&dummy_buf, 0, MPI_INT, target_rank, TAG_STOP, MPI_COMM_WORLD);
                    
                    partner_active = false;
                    received_response = true; // Will break outer loop
                }
            }
        }
    }

    double t_end = MPI_Wtime();
    printf("[Rank %d] Done. Time: %.4f s. Primes: %ld. Stolen: %ld. Shared: %ld.\n", 
           rank, (t_end - t_start), total_primes, tasks_stolen, tasks_shared);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size == 2) run_p2p_node(rank, size);
    else if (rank == 0) std::cout << "Requires exactly 2 ranks." << std::endl;

    MPI_Finalize();
    return 0;
}

/**
mpic++ -o prime_stealing.bin prime_stealing.cpp
mpirun -np 2 ./prime_stealing.bin
 */