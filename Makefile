CXX = g++
AR = ar
FLAG = -std=c++17 -Wall -g

SRCDIR = src
INCDIR = inc
OBJDIR = out
BINDIR = bin

SRC = $(wildcard $(SRCDIR)/*.cc)
OBJ = $(patsubst %.cc, $(OBJDIR)/%.o, $(notdir $(SRC)))

TARGET = libargparse

all: $(BINDIR)/$(TARGET).a

$(BINDIR)/$(TARGET).a: $(OBJ)
	$(AR) -rcs $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(CXX) $(FLAG) -I$(INCDIR) -c -o $@ $<
