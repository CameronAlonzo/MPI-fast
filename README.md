# MPI-fast
Faster MPI optimization of nested loops.
Returns the shortest distance between nodes in a matrix
# Usage:
mpiexec -n <processes> .<filename> <matrixsize>

# Example makefile for UNIX enviornments:
CC = mpicc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm

.PHONY: all clean

all: mpifast

mpifast: diag.c
  $((CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
rm -f mpifast
