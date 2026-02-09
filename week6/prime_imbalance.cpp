#include <mpi.h>
#include <iostream>
#include <vector>

bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) {
            std::cout << "This program requires exactly 2 ranks." << std::endl;
        }
        MPI_Finalize();
        return 0;
    }

    const int MAX_VAL = 10000000;
    const int HALF = MAX_VAL / 2;
    
    int start = (rank == 0) ? 2 : HALF + 1;
    int end = (rank == 0) ? HALF : MAX_VAL;

    double t_start = MPI_Wtime();
    
    int primes = 0;
    for (int i = start; i <= end; i++) {
        if (is_prime(i)) primes++;
    }
    
    double t_local = MPI_Wtime() - t_start;

    std::cout << "Rank " << rank << ": primes = " << primes << std::endl;

    double all_times[2];
    MPI_Gather(&t_local, 1, MPI_DOUBLE, all_times, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::cout << "Performance Results:" << std::endl;
        std::cout << "Rank 0 Time: " << all_times[0] << " sec" << std::endl;
        std::cout << "Rank 1 Time: " << all_times[1] << " sec" << std::endl;
        
        double diff = all_times[1] - all_times[0];
        double idle_pct = (diff / all_times[1]) * 100.0;
        
        std::cout << "Difference:  " << diff << " sec" << std::endl;
        std::cout << "Rank 0 Idle: " << idle_pct << "%" << std::endl;
    }

    MPI_Finalize();
    return 0;
}

/**
mpic++ prime_imbalance.cpp -o prime_imbalance.bin
mpirun -np 2 ./prime_imbalance.bin
 */