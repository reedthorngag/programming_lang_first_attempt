SHELL := bash

all: build

run:
	make build
	./bin/compiler


LIB_PATHS = 
LIBS = 
CFLAGS = -std=c++20 -Wall -W -Werror -Wno-unused-parameter -Wno-unused-variable
DIRS = src/assembly/lexer/ src/assembly/utils/
SRC = $(foreach dir,$(DIRS), $(wildcard $(dir)*.asm))
OBJ_FILES = $(foreach file,$(foreach file,$(SRC), $(notdir $(file))),bin/$(file:.asm=.o))

CPP_DIRS = src/ src/compiler/ src/lexer/ src/parser/ src/typechecker/
CPP_SRC = $(foreach dir,$(CPP_DIRS), $(wildcard $(dir)*.cpp))
CPP_OBJ_FILES = $(foreach file,$(foreach file,$(CPP_SRC), $(notdir $(file))),bin/$(file:.cpp=.o))

build: $(SRC)
	make clean
	make build_cpp

	echo $(OBJ_FILES);
	echo $(SRC);
	for file in $(SRC); do\
		name=$${file%.*}; \
		name=$${name##*/}; \
		nasm -f elf64 -g $$file -o bin/$${name}.o; \
	done

	gcc $(CFLAGS) $(LIB_PATHS) $(OBJ_FILES) $(DEPS_OBJ_FILES) $(LIBS) -o bin/compiler

build_cpp:
	echo $(CPP_SRC)
	for file in $(CPP_SRC); do\
		name=$${file%.*}; \
		name=$${name##*/}; \
		gcc $(CFLAGS) $(LIB_PATHS) -c $$file $(LIBS) -o bin/$${name}.o; \
	done

test: test.cpp
	gcc $(CFLAGS) -I include/ $(LIB_PATHS) -S test.cpp $(LIBS) -o test_out.asm

clean:
	rm -f bin/*.o

