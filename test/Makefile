CXX = g++
FLAG = -std=c++17 -Wall -g

INCDIR = ../inc
LIBDIR = ../bin

LIB = argparse

%.out: %.cc
	$(CXX) $(FLAG) -I$(INCDIR) $^ -L$(LIBDIR) -l$(LIB) -o $@
