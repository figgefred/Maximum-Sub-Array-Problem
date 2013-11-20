#!/bin/bash

# Makefile make me fast and accurate

CC = g++
FLAGS = -Wall -g --std=c++11 -fopenmp -O

SRC_DIR = src/
BIN = bin/

#REF
SRC_REF = msa-ref.cpp
SOURCES_REF = $(SRC_REF:%.cpp=$(SRC_DIR)%.cpp)
OBJ_REF = $(SOURCES_REF:$(SRC_DIR)%.cpp=$(BIN)%.o)
EXE_REF = ref

#THREAD
SRC_THREAD = msa-thread.cpp 
SOURCES_THREAD = $(SRC_THREAD:%.cpp=$(SRC_DIR)%.cpp)
OBJ_THREAD = $(SOURCES_THREAD:$(SRC_DIR)%.cpp=$(BIN)%.o)
EXE_THREAD = thread

#OPENMP
SRC_OPENMP = msa-openmp.cpp 
SOURCES_OPENMP = $(SRC_OPENMP:%.cpp=$(SRC_DIR)%.cpp)
OBJ_OPENMP = $(SOURCES_OPENMP:$(SRC_DIR)%.cpp=$(BIN)%.o)
EXE_OPENMP = openmp

#OPENMP-TASK
SRC_OPENTASK = msa-opentask.cpp 
SOURCES_OPENTASK = $(SRC_OPENTASK:%.cpp=$(SRC_DIR)%.cpp)
OBJ_OPENTASK = $(SOURCES_OPENTASK:$(SRC_DIR)%.cpp=$(BIN)%.o)
EXE_OPENTASK = opentask


#MISC
SRC_MISC = matrix.cpp
SOURCES_MISC = $(SRC_MISC:%.cpp=$(SRC_DIR)%.cpp)
OBJ_MISC = $(SOURCES_MISC:$(SRC_DIR)%.cpp=$(BIN)%.o)
MISC = misc

# All objects... Everything added above should be appended here.
# OBJ = $(OBJ_REF) $(OBJ_THREAD)
EXE = $(EXE_REF) $(EXE_THREAD) $(EXE_OPENMP) $(EXE_OPENTASK)
EXE_ARGS = input/test_input_1000.in
CCFLAGS = -I$(SRC_DIR)

all: $(EXE)

damn:  $(BIN)
	echo "bajs $(SOURCES_THREAD) $(OBJ_THREAD)"
	g++ $(SOURCES_THREAD) src/matrix.h $(FLAGS) -o $(OBJ_THREAD)
	#$(CC) -c $(SOURCES_THREAD) $(CCFLAGS) $(FLAGS) -o $(OBJ_THREAD)




#CLEAN
cleanall:
	rm -rf bin/

clean: 
	rm -f bin/*




#EXECUTABLES

$(EXE_REF): $(OBJ_REF) | $(BIN)
	$(CC) $(OBJ_REF) $(CCFLAGS) $(FLAGS) -o $(BIN)$(EXE_REF)

$(EXE_THREAD): $(OBJ_THREAD) $(OBJ_MISC) | $(BIN)
	$(CC) $(OBJ_THREAD) $(OBJ_MISC) $(CCFLAGS) $(FLAGS) -o $(BIN)$(EXE_THREAD)

$(EXE_OPENMP): $(OBJ_OPENMP) $(OBJ_MISC) | $(BIN)
	$(CC) $(OBJ_OPENMP) $(OBJ_MISC) $(CCFLAGS) $(FLAGS) -o $(BIN)$(EXE_OPENMP)

$(EXE_OPENTASK): $(OBJ_OPENTASK) $(OBJ_MISC) | $(BIN)
	$(CC) $(OBJ_OPENTASK) $(OBJ_MISC) $(CCFLAGS) $(FLAGS) -o $(BIN)$(EXE_OPENTASK)

#OBJECTS
$(OBJ_REF): $(SOURCES_REF) | $(BIN)
	$(CC) -c $(SOURCES_REF) $(CCFLAGS) $(FLAGS) -o $(OBJ_REF)

$(OBJ_THREAD): $(SOURCES_THREAD) | $(BIN)
	$(CC) -c $(SOURCES_THREAD) $(CCFLAGS) $(FLAGS) -o $(OBJ_THREAD)

$(OBJ_OPENMP): $(SOURCES_OPENMP) | $(BIN)
	$(CC) -c $(SOURCES_OPENMP) $(CCFLAGS) $(FLAGS) -o $(OBJ_OPENMP)

$(OBJ_OPENTASK): $(SOURCES_OPENTASK) | $(BIN)
	$(CC) -c $(SOURCES_OPENTASK) $(CCFLAGS) $(FLAGS) -o $(OBJ_OPENTASK)

#MISC
$(MISC): $(OBJ_MISC)

$(OBJ_MISC): $(SOURCES_MISC) | $(BIN)
	$(CC) -c $(SOURCES_MISC) $(CCFLAGS) $(FLAGS) -o $(OBJ_MISC)




#Make BIN?
$(BIN): 
	mkdir -p $(BIN)




#Some run sample
run_$(EXE_REF):
	$(BIN)$(EXE_REF) $(EXE_ARGS)

run_$(EXE_THREAD):
	$(BIN)$(EXE_THREAD) $(EXE_ARGS)
