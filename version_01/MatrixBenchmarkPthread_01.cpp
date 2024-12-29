#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <thread>

template<typename T>
void generate_matrix_element(T** matrix, const size_t ROW, const size_t COL);

template<typename T>
void matrix_product_rc(T** A, T** B, T** C, const size_t ROW, const size_t COL, const size_t START, const size_t END);

// template<typename T>
// void operation_matrix(T** A, T** B, T** C, const size_t ROW, const size_t COL, size_t NUMTHREAD);

template<typename T>
void print_matrix(T** matrix, const size_t ROW, const size_t COL);

template <typename T>
T** allocate_matrix(const size_t ROW, const size_t COL);

template <typename T>
void deallocate_matrix(T** matrix, const size_t ROW);

template<typename T>
void create_operation_matrix(const size_t ROW, const size_t COL);

int main() {
    size_t SIZE = 12;
    create_operation_matrix<int>(SIZE, SIZE);
    return 0;
}

template<typename T>
void generate_matrix_element(T** matrix, const size_t ROW, const size_t COL) {
    std::random_device rd;                          // Obtain a random number from hardware
    std::mt19937 gen(rd());                         // Seed the generator

    if constexpr (std::is_integral<T>::value) {         // random integral type
        std::uniform_int_distribution<T> distr(0, 10);  // Define the range
        for (size_t row = 0; row < ROW; ++row) {
            for (size_t col = 0; col < COL; ++col) {
                matrix[row][col] = distr(gen);
            }
        }
    } else if constexpr (std::is_floating_point<T>::value) { // random floating-point type
        std::uniform_real_distribution<T> distr(0.0, 10.0);  // Define the range
        for (size_t row = 0; row < ROW; ++row) {
            for (size_t col = 0; col < COL; ++col) {
                matrix[row][col] = distr(gen);
            }
        }
    }    
}

template<typename T>
void matrix_product_rc(T** A, T** B, T** C, const size_t ROW, const size_t COL, const size_t START, const size_t END) {
    // Perform matrix multiplication (A * B = C)
    // std::cout << "ID: " << std::this_thread::get_id() << " Start: " << START << " End: " << END << std::endl;
    for (size_t i = START; i < END; ++i) {
        for (size_t j = 0; j < COL; ++j) {
            C[i][j] = 0;
            for (size_t k = 0; k < COL; ++k)  // Use COL for matrix B's row dimension
                C[i][j] += A[i][k] * B[k][j]; // Row × Column multiplication
        }
    }
}

template<typename T>
void operation_matrix(T** A, T** B, T** C, const size_t ROW, const size_t COL, size_t NUMTHREAD = 0) {
    size_t cpu_units = NUMTHREAD == 0 ? std::thread::hardware_concurrency() : NUMTHREAD;
    // std::cout << "Using " << cpu_units << " threads." << std::endl;
    cpu_units = cpu_units - 2; // leaves 2 thread for OS
    
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    size_t rows_per_thread = (ROW <= cpu_units) ? 1 : ((ROW + cpu_units - 1) / cpu_units);
    // std::cout << rows_per_thread << std::endl;
    size_t num_threads = (ROW <= cpu_units) ? ROW : cpu_units;

    for (size_t i = 0; i < num_threads; ++i) {
        size_t start_row = i * rows_per_thread;
        size_t end_row = (i + 1) * rows_per_thread;
        
        if (end_row > ROW) {
            end_row = ROW;
        }

        threads.push_back(
            std::move(
                std::thread([A, B, C, ROW, COL, start_row, end_row]() {
                    matrix_product_rc(A, B, C, ROW, COL, start_row, end_row);
                })
            )
        );
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time_ms = end_time - start_time;
    std::cout << "Processing Time of " << ROW << 'x' << COL << ": " << elapsed_time_ms.count() << " seconds" << std::endl;
}

template<typename T>
void print_matrix(T** matrix, const size_t ROW, const size_t COL) {
    for (size_t row = 0; row < ROW; ++row) {
        for (size_t col = 0; col < COL; ++col) {
            std::cout << std::setw(3) << matrix[row][col] << " ";
        }
        std::cout << std::endl;
    }
}

template <typename T>
T** allocate_matrix(const size_t ROW, const size_t COL) {
    T** matrix = new T*[ROW];
    for (size_t row = 0; row < ROW; ++row) {
        matrix[row] = new T[COL];
    }
    return matrix;
}

template <typename T>
void deallocate_matrix(T** matrix, const size_t ROW) {
    for (size_t row = 0; row < ROW; ++row) {
        delete[] matrix[row];  // Use delete[] for arrays
    }
    delete[] matrix;  // Use delete[] for array of pointers
}

template<typename T>
void create_operation_matrix(const size_t ROW, const size_t COL) {
    T** matrix_A = allocate_matrix<T>(ROW, COL);
    T** matrix_B = allocate_matrix<T>(ROW, COL);
    T** matrix_C = allocate_matrix<T>(ROW, COL);

    // std::cout << "Matrix A:" << std::endl;
    generate_matrix_element(matrix_A, ROW, COL);
    // print_matrix(matrix_A, ROW, COL);

    // std::cout << "Matrix B:" << std::endl;
    generate_matrix_element(matrix_B, ROW, COL);
    // print_matrix(matrix_B, ROW, COL);

    // std::cout << "Matrix C (Product of A and B):" << std::endl;
    // matrix_product_rc(matrix_A, matrix_B, matrix_C, ROW, COL);
    // print_matrix(matrix_C, ROW, COL);

    operation_matrix(matrix_A, matrix_B, matrix_C, ROW, COL);
    print_matrix(matrix_C, ROW, COL);

    deallocate_matrix(matrix_A, ROW);
    deallocate_matrix(matrix_B, ROW);
    deallocate_matrix(matrix_C, ROW);
}
