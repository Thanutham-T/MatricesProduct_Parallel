#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <random>
#include <type_traits>
#include <cxxabi.h>
#include <future>

template <typename T>
T** allocateMatrix(size_t size);

template <typename T>
void deallocateMatrix(T** matrix, size_t size);

template <typename T>
void RandomElements(T** Arr, const size_t NUM_ARR);

template <typename T>
void Print_arr(T** Arr, const size_t NUM_ARR);

template <typename T>
void RC_Product(T** A, T** B, T** C, const size_t NUM_ARR);

template <typename T>
void RR_Product(T** A, T** B, T** C, const size_t NUM_ARR);

template <typename Func, typename T>
double measureExecutionTime(Func func, T** A, T** B, T** C, const size_t NUM_ARR);

void printTimeResult(const double* RC_Time, const double* RR_Time, const int ROUND);

template <typename T>
std::string demangleTypeName();

template <typename MyType>
void runTest(const size_t NUM_ARR, const int ROUND);

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <type> <scale> <round>\n";
        return 1;
    }

    std::string mtype = argv[1];
    const size_t NUM_ARR = static_cast<size_t>(std::stoul(argv[2]));
    const int round = std::stoi(argv[3]);

    if (mtype == "int") {
        runTest<int>(NUM_ARR, round);
    } else if (mtype == "2long") {
        runTest<long long>(NUM_ARR, round);
    } else if (mtype == "float") {
        runTest<float>(NUM_ARR, round);
    } else if (mtype == "double") {
        runTest<double>(NUM_ARR, round);
    } else {
        std::cerr << "Unsupported type: " << mtype << "\n";
        return 1;
    }

    return 0;
}
template <typename T>
T** allocateMatrix(size_t size) {
    T** matrix = new T*[size];
    for (size_t i = 0; i < size; i++) {
        matrix[i] = new T[size];
    }
    return matrix;
}

template <typename T>
void deallocateMatrix(T** matrix, size_t size) {
    for (size_t i = 0; i < size; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

template <typename T>
void RandomElements(T** Arr, const size_t NUM_ARR) {
    std::random_device rd;
    std::default_random_engine gen(rd());
    for (size_t i = 0; i < NUM_ARR; i++) {
        for (size_t j = 0; j < NUM_ARR; j++) {
            if (std::is_integral<T>::value) {
                if constexpr (std::is_same<T, int>::value) {
                    std::uniform_int_distribution<int> dis(0, 99);
                    Arr[i][j] = dis(gen);
                } else if constexpr (std::is_same<T, long long>::value) {
                    std::uniform_int_distribution<long long> dis(0, 99);
                    Arr[i][j] = dis(gen);
                }
            } else if (std::is_floating_point<T>::value) {
                if constexpr (std::is_same<T, float>::value) {
                    std::uniform_real_distribution<float> dis(0.0f, 99.0f);
                    Arr[i][j] = dis(gen);
                } else if constexpr (std::is_same<T, double>::value) {
                    std::uniform_real_distribution<double> dis(0.0, 99.0);
                    Arr[i][j] = dis(gen);
                }
            }
        }
    }
}

template <typename T>
void Print_arr(T** Arr, const size_t NUM_ARR) {
    for (size_t i = 0; i < NUM_ARR; i++) {
        for (size_t j = 0; j < NUM_ARR; j++) {
            std::cout << Arr[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

template <typename T>
void RC_Product(T** A, T** B, T** C, const size_t NUM_ARR) {
    for (size_t i = 0; i < NUM_ARR; i++) {
        for (size_t j = 0; j < NUM_ARR; j++) {
            T sum = 0;
            for (size_t k = 0; k < NUM_ARR; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

template <typename T>
void RR_Product(T** A, T** B, T** C, const size_t NUM_ARR) {
    for (size_t i = 0; i < NUM_ARR; i++) {
        for (size_t j = 0; j < NUM_ARR; j++) {
            T sum = 0;
            for (size_t k = 0; k < NUM_ARR; k++) {
                sum += A[i][k] * B[j][k];
            }
            C[i][j] = sum;
        }
    }
}

template <typename Func, typename T>
double measureExecutionTime(Func func, T** A, T** B, T** C, const size_t NUM_ARR) {
    auto future = std::async(std::launch::async, func, A, B, C, NUM_ARR);
    auto start_time = std::chrono::high_resolution_clock::now();
    future.get();
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    return duration.count();
}

void printTimeResult(const double* RC_Time, const double* RR_Time, const int ROUND) {
    double RC_avg_time = 0, RR_avg_time = 0;
    for (int i = 0; i < ROUND; i++) {
        RC_avg_time += RC_Time[i];
        RR_avg_time += RR_Time[i];
    }
    std::cout << "Summary: " << std::endl;
    RC_avg_time /= ROUND;
    RR_avg_time /= ROUND;
    std::cout << "Average Execution Time for RC product: " << RC_avg_time << " seconds" << std::endl;
    std::cout << "Average Execution Time for RR product: " << RR_avg_time << " seconds" << std::endl;
    double diff = abs(RC_avg_time - RR_avg_time);
    std::cout << "Difference: " << diff << " seconds" << std::endl;
    std::cout << "Speedup: " << RC_avg_time / RR_avg_time << std::endl;
}

template <typename T>
std::string demangleTypeName() {
    int status = 0;
    char* demangled_name = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string result = (status == 0) ? demangled_name : typeid(T).name();
    free(demangled_name);
    return result;
}

template <typename MyType>
void runTest(const size_t NUM_ARR, const int ROUND) {
    MyType** A = allocateMatrix<MyType>(NUM_ARR);
    MyType** B = allocateMatrix<MyType>(NUM_ARR);
    MyType** C_RC = allocateMatrix<MyType>(NUM_ARR);
    MyType** C_RR = allocateMatrix<MyType>(NUM_ARR);

    double RC_Time[ROUND], RR_Time[ROUND];
    bool swap_flag = false;

    std::cout << "TESTING {size:" << NUM_ARR << ", type:" << demangleTypeName<MyType>() << "}" << std::endl;

    for (int i = 0; i < ROUND; i++) {
        RandomElements(A, NUM_ARR);
        RandomElements(B, NUM_ARR);

        if (swap_flag) {
            RC_Time[i] = measureExecutionTime(RC_Product<MyType>, A, B, C_RC, NUM_ARR);
            RR_Time[i] = measureExecutionTime(RR_Product<MyType>, A, B, C_RR, NUM_ARR);
        } else {
            RR_Time[i] = measureExecutionTime(RR_Product<MyType>, A, B, C_RR, NUM_ARR);
            RC_Time[i] = measureExecutionTime(RC_Product<MyType>, A, B, C_RC, NUM_ARR);
        }
        std::cout << "Round " << i + 1 << ":" << std::endl;
        std::cout << "Execution Time RC product: " << RC_Time[i] << " seconds" << std::endl;
        std::cout << "Execution Time RR product: " << RR_Time[i] << " seconds" << std::endl;
        swap_flag = !swap_flag;
    }

    printTimeResult(RC_Time, RR_Time, ROUND);

    deallocateMatrix(A, NUM_ARR);
    deallocateMatrix(B, NUM_ARR);
    deallocateMatrix(C_RC, NUM_ARR);
    deallocateMatrix(C_RR, NUM_ARR);
}