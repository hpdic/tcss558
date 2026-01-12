/*
 * Author: Dongfang Zhao
 * Email: dzhao@uw.edu
 */

#include <cstdio>
#include <iostream>
#include <mpi.h>

int main(int argc, char **argv) {
  // 1. Initialize MPI
  MPI_Init(&argc, &argv);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // We define a constant TAG to label our "envelope"
  const int PING_TAG = 0;

  // 2. The Logic: Rank 0 talks to Rank 1
  if (world_rank == 0) {
    // --- SENDER ---
    int secret_data = 999;

    printf("[Rank 0] Preparing to send data: %d\n", secret_data);

    // MPI_Send(data_ptr, count, type, dest, tag, communicator)
    // Blocking Send: This might wait until Rank 1 is ready to receive.
    MPI_Send(&secret_data, 1, MPI_INT, 1, PING_TAG, MPI_COMM_WORLD);

    printf("[Rank 0] Data sent successfully.\n");

  } else if (world_rank == 1) {
    // --- RECEIVER ---
    int received_buffer;

    // MPI_Recv(data_ptr, count, type, source, tag, communicator, status)
    // Blocking Recv: The program STOPS here until a message arrives.
    MPI_Recv(&received_buffer, 1, MPI_INT, 0, PING_TAG, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    printf("[Rank 1] I received the secret data: %d\n", received_buffer);

  } else {
    // --- BYSTANDERS ---
    printf("[Rank %d] I have nothing to do today.\n", world_rank);
  }

  // 3. Finalize
  MPI_Finalize();
  return 0;
}

/**
 * To setup the system
 *      On Ubuntu: sudo apt install openmpi-bin libopenmpi-dev build-essential
 *      On MacOS: brew install open-mpi
 * To compile: mpic++ simple_p2p.cpp -o simple_p2p.bin
 * To run: mpirun -np 4 ./simple_p2p.bin
 */