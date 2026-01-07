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

  // 1. Setup Data
  const int N = 1000;
  // Calculate how many elements each process will handle.
  // NOTE: For simplicity, we assume N is divisible by world_size (1000 / 4 =
  // 250). In real code, you need MPI_Scatterv for uneven distribution.
  int elements_per_proc = N / world_size;

  // The Master process (Rank 0) holds the full original data.
  std::vector<int> global_data;

  if (world_rank == 0) {
    // Initialize the vector with 0, 1, 2, ..., 999
    global_data.resize(N);
    std::iota(global_data.begin(), global_data.end(), 0);
    printf("[Master] Initialized vector with %d elements.\n", N);
  }

  // 2. Allocate Local Buffer
  // Every process (including Master) needs a small bucket to catch its share.
  std::vector<int> local_data(elements_per_proc);

  // 3. SCATTER: Distribute data from Master to everyone
  // MPI_Scatter(send_buf, send_count_per_process, send_type,
  //             recv_buf, recv_count, recv_type, root, comm)
  MPI_Scatter(global_data.data(), elements_per_proc, MPI_INT, local_data.data(),
              elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

  // 4. COMPUTE: Distributed Processing
  // Each node multiplies its own 250 elements by 2.
  // This happens simultaneously on all 4 nodes!
  printf("[Rank %d] Received %d elements. First element is %d. Processing...\n",
         world_rank, elements_per_proc, local_data[0]);

  for (int i = 0; i < elements_per_proc; i++) {
    local_data[i] *= 2;
  }

  // 5. GATHER: Collect results back to Master
  // MPI_Gather(send_buf, send_count, send_type,
  //            recv_buf, recv_count, recv_type, root, comm)
  MPI_Gather(local_data.data(), elements_per_proc, MPI_INT, global_data.data(),
             elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

  // 6. Verification (Master only)
  if (world_rank == 0) {
    printf("[Master] All data collected.\n");
    printf("Verification:\n");
    printf("  Index 0:   Expected 0,   Got %d\n", global_data[0]);
    printf("  Index 250: Expected 500, Got %d\n",
           global_data[250]); // Start of Rank 1's work
    printf("  Index 999: Expected 1998, Got %d\n", global_data[999]);
  }

  MPI_Finalize();
  return 0;
}

/**
 * To setup the system
 * On Ubuntu: sudo apt install openmpi-bin libopenmpi-dev build-essential
 * On MacOS: brew install open-mpi
 * To compile: mpic++ vector_multiply.cpp -o vector_mult.bin
 * To run
 * Single process: ./vector_mult.bin (Will work, but scatter is trivial)
 * Multiple processes: mpirun -np 4 ./vector_mult.bin
 */