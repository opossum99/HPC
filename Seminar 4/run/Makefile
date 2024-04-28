GCC = nvcc

VPATH = ../:../glad

CUDA_SOURCES = main.cu
C_SOURCES = glad.c

all: prog.exe

prog.exe: $(CUDA_SOURCES) $(C_SOURCES)
	$(GCC) -Xcudafe="--diag_suppress=esa_on_defaulted_function_ignored" $^ -lm -ldl -lglfw -o $@

clean:
	rm -rf *exe
