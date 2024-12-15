# MatricesProduct_Parallel

using C++ POSIX(pthread) and Async key word.
Compare 2 method of product between Row x Row and Row x Column by the execution time using chrono in seconds.

using CLI input in format <type> <scale> <round>
there are 4 types:
  - int -> int
  - 2long -> long long
  - float -> float
  - double -> double
scale is size of matrices and round is round for testing.
At the end It will calculate the average time for each method and find difference between them.
