TARGET_SPM = bin/spm_test
SRC_SPM = src/spm_test.c
INCLUDES = -Iinclude

CC = clang
CFLAGS = --target=riscv32 -march=rv32im -mabi=ilp32 -nostdlib -static -fno-builtin -fuse-ld=lld -T linker.ld -g

GEM5_ROOT = /home/xuwenyi/gem5
GEM5_BIN = $(GEM5_ROOT)/build/RISCV/gem5.opt
GEM5_SCRIPT = $(GEM5_ROOT)/configs/tutorial/part1/l1d_spm_test.py

all: $(TARGET_SPM)


$(TARGET_SPM): $(SRC_SPM)
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^
	llvm-objdump -S -d $@ > $@.dump

clean:
	rm -rf bin

run-spm: $(TARGET_SPM)
	$(GEM5_BIN) --outdir=$(GEM5_ROOT)/m5out $(GEM5_SCRIPT)


visualize:
	$(GEM5_ROOT)/util/o3-pipeview.py -c 1000 -o $(GEM5_ROOT)/m5out/pipeview.out --color $(GEM5_ROOT)/m5out/trace.out
	@echo "Visualization generated at $(GEM5_ROOT)/m5out/pipeview.out"
	@echo "View it with: less -r $(GEM5_ROOT)/m5out/pipeview.out"
