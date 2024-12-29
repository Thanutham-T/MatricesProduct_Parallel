> [!WARNING]
> to compiled OMP using `g++ -std=c++20 <filename.cpp> -o <output.out> -fopenmd`, to compiled MPI + OMP using `mpic++ <filename.cpp> -o <output.out> -fopenmd`.

> [!NOTE]
> The execute files get CLI input(OMP) Usage: `./output.out <type> <scale> <round> <product_method(rc,rr)>`, The execute files get CLI input(MPI+OPENMP) Usage: `mpirun ./output.out <type> <scale> <round> <product_method(rc,rr)>`.
