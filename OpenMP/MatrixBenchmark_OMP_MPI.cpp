#include <iostream>
#include <omp.h>
#include <mpi.h>
#include <vector>
#include <chrono>
#include <random>

// Function to generate matrix elements
template<typename T>
void generate_matrix_element(T* matrix, size_t ROW, size_t COL) {
    std::random_device rd;
    std::mt19937 gen(rd());
    if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> distr(0, 10);
        for (size_t i = 0; i < ROW * COL; ++i)
            matrix[i] = distr(gen);
    } else if constexpr (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> distr(0.0, 10.0);
        for (size_t i = 0; i < ROW * COL; ++i)
            matrix[i] = distr(gen);
    }
}

// Function to multiply matrices
template <typename T>
void matrix_product_rc(T* A, T* B, T* C, const size_t ROW, const size_t COL) {
    #pragma omp parallel for
    for (size_t i = 0; i < ROW; ++i) {
        for (size_t j = 0; j < COL; ++j) {
            C[i * COL + j] = 0;
            for (size_t k = 0; k < COL; ++k) {
                C[i * COL + j] += A[i * COL + k] * B[k * COL + j];
            }
        }
    }
}

int main(int argc, char** argv) {
    auto start_time = std::chrono::high_resolution_clock::now();
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const size_t N = 8192;
    size_t rows_per_process = (N + size - 1) / size;
    size_t padded_rows = rows_per_process * size;

    double* A = nullptr;
    double* B = nullptr;
    double* C = nullptr;

    if (rank == 0) {
        A = new double[N * N];
        B = new double[N * N];
        C = new double[N * N]();

        generate_matrix_element(A, N, N);
        generate_matrix_element(B, N, N);
    }

    double* local_A = new double[rows_per_process * N]();
    double* local_C = new double[rows_per_process * N]();

    MPI_Scatter(A, rows_per_process * N, MPI_DOUBLE, local_A, rows_per_process * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } else {
        B = new double[N * N];
        MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    matrix_product_rc(local_A, B, local_C, rows_per_process, N);

    MPI_Gather(local_C, rows_per_process * N, MPI_DOUBLE, C, rows_per_process * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // std::cout << "Resultant Matrix C:" << std::endl;
        // for (size_t i = 0; i < N; ++i) {
        //     for (size_t j = 0; j < N; ++j) {
        //         std::cout << C[i * N + j] << " ";
        //     }
        //     std::cout << std::endl;
        // }
        delete[] A;
        delete[] B;
        delete[] C;
    }

    delete[] local_A;
    delete[] local_C;
    if (rank != 0) {
        delete[] B;
    }

    MPI_Finalize();

    auto end_time = std::chrono::high_resolution_clock::now();

    // Timing calculations
    if (rank == 0) {
        std::cout << "Total Execution: " << std::chrono::duration<double>(end_time - start_time).count() << std::endl;
    }

    return 0;
}
