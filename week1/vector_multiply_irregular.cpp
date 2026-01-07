/*
 * Author: Dongfang Zhao
 * Email: dzhao@uw.edu
 */

#include <cstdio>
#include <iostream>
#include <mpi.h>
#include <numeric> // For std::iota
#include <vector>

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int world_rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  const int N = 1003; // Irregular size, not divisible by 4

  // Variables to hold the distribution logic (Only significant on Root)
  std::vector<int> send_counts;
  std::vector<int> displs;
  std::vector<int> global_data;

  if (world_rank == 0) {
    // 1. Data Generation
    global_data.resize(N);
    std::iota(global_data.begin(), global_data.end(), 0);

    // 2. Manual Distribution Logic (The "%" Logic)
    send_counts.resize(world_size);
    displs.resize(world_size);

    int base_count = N / world_size; // 1003 / 4 = 250
    int remainder = N % world_size;  // 1003 % 4 = 3

    int current_idx = 0;
    for (int i = 0; i < world_size; i++) {
      // Distribute remainder: The first 'remainder' ranks get +1 item
      send_counts[i] = base_count + (i < remainder ? 1 : 0);

      // Calculate offset (displacement)
      displs[i] = current_idx;

      // Update current index for the next rank
      current_idx += send_counts[i];

      printf("[Master] Plan for Rank %d: count = %d, start_index = %d\n", i,
             send_counts[i], displs[i]);
    }
  }

  // 3. Every process needs to know how many items to receive
  // We could broadcast the whole 'send_counts' array, but for efficiency,
  // let's just calculate our own expected count locally using the same logic.
  int base_count = N / world_size;
  int remainder = N % world_size;
  int my_count = base_count + (world_rank < remainder ? 1 : 0);

  // 4. Allocate local buffer
  std::vector<int> local_data(my_count);

  // 5. SCATTERV (Vector Scatter)
  // Unlike Scatter, Scatterv allows varying counts and displacements.
  // Notice we pass 'send_counts.data()' and 'displs.data()'
  MPI_Scatterv(global_data.data(), send_counts.data(), displs.data(), MPI_INT,
               local_data.data(), my_count, MPI_INT, 0, MPI_COMM_WORLD);

  // 6. Compute (Multiply by 2)
  printf("[Rank %d] Received %d items. Processing...\n", world_rank, my_count);
  for (int i = 0; i < my_count; i++) {
    local_data[i] *= 2;
  }

  // 7. GATHERV (Vector Gather)
  // We must put them back exactly how we took them.
  MPI_Gatherv(local_data.data(), my_count, MPI_INT, global_data.data(),
              send_counts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

  // 8. Verification
  if (world_rank == 0) {
    printf("[Master] Verification:\n");
    // Verify the last element (Index 1002) which was handled by Rank 3 (or last
    // rank) If N=1003, last index is 1002. value should be 2004.
    printf("  Item [0]    (Rank 0): Expected 0, Got %d\n", global_data[0]);
    printf("  Item [1002] (Rank %d): Expected 2004, Got %d\n", world_size - 1,
           global_data[1002]);
  }

  MPI_Finalize();
  return 0;
}

/**
 * To setup the system
 * On Ubuntu: sudo apt install openmpi-bin libopenmpi-dev build-essential
 * On MacOS: brew install open-mpi
 * To compile: mpic++ vector_multiply_irregular.cpp -o vector_irr.bin
 * To run
 * Multiple processes: mpirun -np 4 ./vector_irr.bin
 */