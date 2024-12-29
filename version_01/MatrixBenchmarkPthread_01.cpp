#include <iostream>
#include <iomanip>
#include <random>

template<typename T>
void generate_matrix_element(T** matrix, const size_t ROW, const size_t COL);

template<typename T>
void matrix_product_rc(T** A, T** B, T** C, const size_t ROW, const size_t COL);

template<typename T>
void print_matrix(T** matrix, const size_t ROW, const size_t COL);

template <typename T>
T** allocate_matrix(const size_t ROW, const size_t COL);

template <typename T>
void deallocate_matrix(T** matrix, const size_t ROW);

template<typename T>
void create_operation_matrix(const size_t ROW, const size_t COL);

int main() {
    create_operation_matrix<int>(8, 8);
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
void matrix_product_rc(T** A, T** B, T** C, const size_t ROW, const size_t COL) {
    // Perform matrix multiplication (A * B = C)
    for (size_t i = 0; i < ROW; ++i) {
        for (size_t j = 0; j < COL; ++j) {
            C[i][j] = 0;
            for (size_t k = 0; k < COL; ++k)  // Use COL for matrix B's row dimension
                C[i][j] += A[i][k] * B[k][j]; // Row × Column multiplication
        }
    }
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

    std::cout << "Matrix A:" << std::endl;
    generate_matrix_element(matrix_A, ROW, COL);
    print_matrix(matrix_A, ROW, COL);

    std::cout << "Matrix B:" << std::endl;
    generate_matrix_element(matrix_B, ROW, COL);
    print_matrix(matrix_B, ROW, COL);

    std::cout << "Matrix C (Product of A and B):" << std::endl;
    matrix_product_rc(matrix_A, matrix_B, matrix_C, ROW, COL);
    print_matrix(matrix_C, ROW, COL);

    deallocate_matrix(matrix_A, ROW);
    deallocate_matrix(matrix_B, ROW);
    deallocate_matrix(matrix_C, ROW);
}
