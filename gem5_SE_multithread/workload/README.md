statically compile and link the pthread C++ program:
    riscv32-unknown-linux-gnu-g++ -g --verbose gemm_pthread.cpp -o gemm_pthread.out -static

./randomMatrix.py: a python script to generate matrices and their product, used to verify the functionality correctness of pthread C++ program. In C++, decomment the file output snippets enclosed in /**/ and include the fstream header.

note: currently the dynamic link loading issue is not resolved in Gem5 SE mode
