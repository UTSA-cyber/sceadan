echo testing for openmp with lots of different compiler installs
gcc -fopenmp -o openmp_test-gcc openmp_test.c
gcc-mp-4.8 -fopenmp -o openmp_test-gcc-mp-4.8 openmp_test.c
clang -fopenmp -o openmp_test-clang openmp_test.c
