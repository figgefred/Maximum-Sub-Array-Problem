#!/bin/bash

# Makefile make me fast and accurate

CC = g++
BIN = bin

EXE_REF = ref 
EXE_COL = col
EXE_ROW = row 
EXE = $(EXE_REF) $(EXE_ROW) $(EXE_COL)
EXE_ARGS = input/test_input_01.in

CFLAGS = -Wall -g -std=c++11 -fopenmp
LFLAGS = 

SRC_REF =  msa-vla_ref.cpp
SRC_ROW =  msa-vla_row.cpp
SRC_COL =  msa-vla_col.cpp

OBJ_REF = $(SRC_REF:.cpp=.o)
OBJ_ROW = $(SRC_ROW:.cpp=.o)
OBJ_COL = $(SRC_COL:.cpp=.o)
OBJ = $(OBJ_REF) $(OBJ_ROW) $(OBJ_COL)

.PHONY: clean

all: $(EXE)

$(EXE_REF): $(OBJ_REF)
	$(CC) $(BIN)/$(OBJ_REF) $(CFLAGS) $(LFLAGS) -o $(BIN)/$(EXE_REF)

$(EXE_COL): $(OBJ_COL)
	$(CC) $(BIN)/$(OBJ_COL) $(CFLAGS) $(LFLAGS) -o $(BIN)/$(EXE_COL)

$(EXE_ROW): $(OBJ_ROW)
	$(CC) $(BIN)/$(OBJ_ROW) $(CFLAGS) $(LFLAGS) -o $(BIN)/$(EXE_ROW)

$(OBJ_REF):
	$(CC) -c $(SRC_REF) $(CFLAGS) -o $(BIN)/$(OBJ_REF)

$(OBJ_ROW):
	$(CC) -c $(SRC_ROW) $(CFLAGS) -o $(BIN)/$(OBJ_ROW)

$(OBJ_COL):
	$(CC) -c $(SRC_COL) $(CFLAGS) -o $(BIN)/$(OBJ_COL)
	

clean: 
	rm -rf $(BIN)/*

run_ref: $(EXE_REF)
	$(BIN)/$(EXE_REF) $(EXE_ARGS)

run_row: $(EXE_ROW)
	$(BIN)/$(EXE_ROW) $(EXE_ARGS)

run_col: $(EXE_COL)
	$(BIN)/$(EXE_COL) $(EXE_ARGS)

# End of Makefile



