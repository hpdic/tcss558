/*
 * File:    deadlock_demo.cpp
 *
 * Purpose: Demonstration of MPI Deadlock (Head-to-Head)
 * Shows how a circular dependency occurs when both ranks try to
 * Send before Receiving using blocking operations.
 *
 * Author:  dzhao@uw.edu
 * Date:    2026-01-14
 * Course:  TCSS 558
 */

#include <mpi.h>
#include <cstdio>
#include <vector>

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Requirement: Exactly 2 processes for this demo
  if (size != 2) {
    if (rank == 0) {
      printf("Error: This program requires exactly 2 processes.\n");
    }
    MPI_Finalize();
    return 0;
  }

  // 1. Data Setup
  // We use a large buffer (N=10000) to force the "Rendezvous Protocol".
  // If the message is small (Eager Protocol), MPI might buffer it internally
  // and hide the deadlock. We want to guarantee the hang.
  const int N = 10000;
  std::vector<int> send_buf(N, rank + 1);
  std::vector<int> recv_buf(N, 0);

  // 2. The Deadlock Trap
  // Both ranks attempt to SEND first.
  // Since MPI_Send is blocking (for large messages), they both wait for
  // the other to call MPI_Recv. Neither ever reaches the Recv line.
  
  if (rank == 0) {
    printf("[Rank 0] Attempting to SEND to Rank 1...\n");
    // BLOCKED HERE: Waiting for Rank 1 to Recv...
    MPI_Send(send_buf.data(), N, MPI_INT, 1, 0, MPI_COMM_WORLD);
    
    // This line is never reached
    printf("[Rank 0] Send passed! Attempting to RECV...\n");
    MPI_Recv(recv_buf.data(), N, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
  } else if (rank == 1) {
    printf("[Rank 1] Attempting to SEND to Rank 0...\n");
    // BLOCKED HERE: Waiting for Rank 0 to Recv...
    MPI_Send(send_buf.data(), N, MPI_INT, 0, 0, MPI_COMM_WORLD);
    
    // This line is never reached
    printf("[Rank 1] Send passed! Attempting to RECV...\n");
    MPI_Recv(recv_buf.data(), N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  printf("[Rank %d] Exchange Complete. (You will unlikely see this)\n", rank);

  MPI_Finalize();
  return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ deadlock_demo.cpp -o deadlock_demo.bin
 *
 * 2. Run (Expect it to HANG):
 * mpirun -np 2 ./deadlock_demo.bin
 *
 * Note: The program will print the "Attempting to SEND" messages
 * and then freeze. You must use 'Ctrl + C' to terminate it.
 * ============================================================
 */