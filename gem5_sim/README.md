How to run GEM5 with SPM:

$ cd gem5_sim/tests/test-progs/spm_test/
$ make
$ cd ../../../
$ scons ./build/ALL/gem5.opt -j8
$ ./build/ALL/gem5.opt --debug-flags=DMA,ScratchpadMemory,MemoryAccess>simple.txt configs/tutorial/part1/l1d_spm_test.py