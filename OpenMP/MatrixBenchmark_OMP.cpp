#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <thread>
#include <omp.h>
#include <functional>

// Function to generate random matrix elements
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

// Parallel matrix multiplication(Row x Column) (A * B = C) using OpenMP
template<typename T>
void matrix_product_rc(T** A, T** B, T** C, const size_t ROW, const size_t COL, size_t NUMTHREAD=0) {
    // std::cout << "ID: " << std::this_thread::get_id() << " Start: " << START << " End: " << END << std::endl;
    int i,j,k;
    size_t cpu_units = omp_get_max_threads();
    // std::cout << "Using " << cpu_units << " threads." << std::endl;
    cpu_units = (cpu_units > 2) ? cpu_units - 2 : 1; // leaves 1-2 thread for OS

    #pragma omp parallel for shared(A, B, C) private(i, j, k) schedule(static) num_threads(cpu_units)
    for (i = 0; i < ROW; ++i) {
        for (j = 0; j < COL; ++j) {
            C[i][j] = 0;
            for (k = 0; k < COL; ++k)  // Use COL for matrix B's row dimension
                C[i][j] += A[i][k] * B[k][j]; // Row × Column multiplication
        }
    }
}

// Parallel matrix multiplication(Row x Row) (A * B = C) using OpenMP
template<typename T>
void matrix_product_rr(T** A, T** B, T** C, const size_t ROW, const size_t COL, size_t NUMTHREAD=0) {
    // std::cout << "ID: " << std::this_thread::get_id() << " Start: " << START << " End: " << END << std::endl;
    int i,j,k;
    size_t cpu_units = omp_get_max_threads();
    // std::cout << "Using " << cpu_units << " threads." << std::endl;
    cpu_units = (cpu_units > 2) ? cpu_units - 2 : 1; // leaves 1-2 thread for OS

    #pragma omp parallel for shared(A, B, C) private(i, j, k) schedule(static) num_threads(cpu_units)
    for (i = 0; i < ROW; ++i) {
        for (j = 0; j < COL; ++j) {
            C[i][j] = 0;
            for (k = 0; k < COL; ++k)  // Use COL for matrix B's row dimension
                C[i][j] += A[i][k] * B[j][k]; // Row × Row multiplication
        }
    }
}

// Function for matrix Operation Timer the calculation time
template<typename T>
double operation_matrix(T** A, T** B, T** C, const size_t ROW, const size_t COL, const std::string& method = "") {

    std::chrono::duration<double> elapsed_time_ms;

    if (method == "rc") {

        auto start_time = std::chrono::high_resolution_clock::now();
        matrix_product_rc(A, B, C, ROW, COL);
        auto end_time = std::chrono::high_resolution_clock::now();
        elapsed_time_ms = end_time - start_time;
        std::cout << "[" << typeid(T).name() << "]";
        std::cout << "Processing Time of " << ROW << 'x' << COL << ": " << elapsed_time_ms.count() << " seconds" << std::endl;

    } else if (method == "rr") {

        auto start_time = std::chrono::high_resolution_clock::now();
        matrix_product_rc(A, B, C, ROW, COL);
        auto end_time = std::chrono::high_resolution_clock::now();
        elapsed_time_ms = end_time - start_time;
        std::cout << "[" << typeid(T).name() << "]";
        std::cout << "Processing Time of " << ROW << 'x' << COL << ": " << elapsed_time_ms.count() << " seconds" << std::endl; 

    } else {
        std::cerr << "Invalid method specified." << std::endl;
    }

    return elapsed_time_ms.count();
}

// Print matrix
template<typename T>
void print_matrix(T** matrix, const size_t ROW, const size_t COL) {
    for (size_t row = 0; row < ROW; ++row) {
        for (size_t col = 0; col < COL; ++col) {
            std::cout << std::setw(3) << matrix[row][col] << " ";
        }
        std::cout << std::endl;
    }
}

// Function matrix allocate heap memory for 2D Array
template <typename T>
T** allocate_matrix(const size_t ROW, const size_t COL) {
    T** matrix = new T*[ROW];
    for (size_t row = 0; row < ROW; ++row) {
        matrix[row] = new T[COL];
    }
    return matrix;
}

// Function matrix deallocate heap memory for 2D Array
template <typename T>
void deallocate_matrix(T** matrix, const size_t ROW) {
    for (size_t row = 0; row < ROW; ++row) {
        delete[] matrix[row];  // Use delete[] for arrays
    }
    delete[] matrix;  // Use delete[] for array of pointers
}

// Function to create matrix and run matrix operation
template<typename T>
void create_operation_matrix(const size_t ROW, const size_t COL, double& sum_times, const std::string& method = "") {
    T** matrix_A = allocate_matrix<T>(ROW, COL);
    T** matrix_B = allocate_matrix<T>(ROW, COL);
    T** matrix_C = allocate_matrix<T>(ROW, COL);

    generate_matrix_element(matrix_A, ROW, COL);
    generate_matrix_element(matrix_B, ROW, COL);

    sum_times += operation_matrix(matrix_A, matrix_B, matrix_C, ROW, COL, method);

    deallocate_matrix(matrix_A, ROW);
    deallocate_matrix(matrix_B, ROW);
    deallocate_matrix(matrix_C, ROW);
}

int main(int argc, char* argv[]) {

    if (argc < 5 || argc > 5) {
        std::cerr << "Usage: " << argv[0] << " <type> <scale> <round> <product_method(rc,rr)>" << std::endl;
        return 1;
    }

    std::string mtype = argv[1];
    const size_t SIZE = static_cast<size_t>(std::stoul(argv[2]));
    const int ROUND = std::stoi(argv[3]);
    std::string method = argv[4];

    double sum_times = 0;

    for(int round = 0; round < ROUND; ++round) {
        std::cout << "ROUND[" << (round+1) << "]: ";
        if (mtype == "int") {
            create_operation_matrix<int>(SIZE, SIZE, sum_times, method);
        } else if (mtype == "2long") {
            create_operation_matrix<long long>(SIZE, SIZE, sum_times, method);
        } else if (mtype == "float") {
            create_operation_matrix<float>(SIZE, SIZE, sum_times, method);
        } else if (mtype == "double") {
            create_operation_matrix<double>(SIZE, SIZE, sum_times, method);
        } else {
            std::cerr << "Unsupported type: " << mtype << "\n";
            return 1;
        }
        if(round < (ROUND-1))
            std::this_thread::sleep_for(std::chrono::seconds(7));
    }

    std::cout << "[" << method << "]" << "Average time: " << (sum_times / ROUND) << " seconds" << std::endl;

    return 0;
}