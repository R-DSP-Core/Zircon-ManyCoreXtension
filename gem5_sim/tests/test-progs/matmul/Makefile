TARGET = bin/main
SRC = src/main.c src/DSP_mat_mul_cn.c
INCLUDES = -Iinclude

CC = clang
CFLAGS = --target=riscv64-unknown-linux-gnu -march=rv64im -mabi=lp64 -nostdlib -static -fuse-ld=lld

GEM5_ROOT = /home/xuwenyi/gem5
GEM5_BIN = $(GEM5_ROOT)/build/RISCV/gem5.opt
GEM5_SCRIPT = $(GEM5_ROOT)/configs/tutorial/part1/four_core_stdlib.py

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o0 $@ $^

clean:
	rm -rf bin

run: $(TARGET)
	$(GEM5_BIN) --outdir=$(GEM5_ROOT)/m5out $(GEM5_SCRIPT) $(GEM5_ROOT)/tests/test-progs/matmul/$(TARGET)

run-trace: $(TARGET)
	$(GEM5_BIN) --debug-flags=O3PipeView --debug-file=trace.out --outdir=$(GEM5_ROOT)/m5out $(GEM5_SCRIPT) $(GEM5_ROOT)/tests/test-progs/matmul/$(TARGET)

visualize:
	$(GEM5_ROOT)/util/o3-pipeview.py -c 1000 -o $(GEM5_ROOT)/m5out/pipeview.out --color $(GEM5_ROOT)/m5out/trace.out
	@echo "Visualization generated at $(GEM5_ROOT)/m5out/pipeview.out"
	@echo "View it with: less -r $(GEM5_ROOT)/m5out/pipeview.out"
