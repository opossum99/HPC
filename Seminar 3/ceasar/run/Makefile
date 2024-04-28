GCC = mpicxx

VPATH = ../
DIR = ./build

SOURCES = main.cpp cipher.cpp
OBJECTS = $(SOURCES:%.cpp=$(DIR)/%.o)

all: prog.exe

prog.exe: $(OBJECTS)
	$(GCC) $^ -o $@

$(DIR)/%.o: %.cpp
	$(GCC) -c $< -o $@


clean:
	rm -rf *.exe $(DIR)/*.o
