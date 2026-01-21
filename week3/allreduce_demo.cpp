/*
 * File:    allreduce_demo.cpp
 *
 * Purpose: Demonstrates MPI_Allreduce.
 * 1. Every process computes a local partial sum.
 * 2. MPI_Allreduce combines them into a Global Sum.
 * 3. EVERY process receives this Global Sum.
 *
 * Author:  dzhao@uw.edu
 * Date:    2026-01-21
 * Course:  TCSS 558
 */

#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 1. Local Work
    int local_val = rank + 1; 
    int global_sum = 0;

    printf("[Rank %d] My local contribution: %d\n", rank, local_val);

    // 2. The All-in-One Step
    // MPI_Reduce: Only Rank 0 gets the result.
    // MPI_Allreduce: EVERYONE gets the result.
    //
    // No 'root' argument needed because the destination is "Everyone".
    MPI_Allreduce(
        &local_val,      // Send buffer (My contribution)
        &global_sum,     // Recv buffer (Where the final total goes)
        1,               // Count
        MPI_INT,         // Datatype
        MPI_SUM,         // Operation
        MPI_COMM_WORLD   // Communicator
    );

    // 3. Independent Decision Making
    // Since everyone has 'global_sum', everyone can make the same decision
    // without waiting for Rank 0 to tell them what to do.
    
    // Normalize my value based on the global total
    double my_percentage = (double)local_val / global_sum * 100.0;
    
    printf("[Rank %d] I know the Total is %d. My share is %.2f%%\n", 
           rank, global_sum, my_percentage);

    MPI_Finalize();
    return 0;
}

/*
 * ============================================================
 * Compile & Run Instructions:
 * ============================================================
 * 1. Compile:
 * mpic++ allreduce_demo.cpp -o allreduce_demo.bin
 *
 * 2. Run:
 * mpirun -np 4 ./allreduce_demo.bin
 * ============================================================
 */