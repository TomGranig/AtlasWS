# Makefile for building the AtlasWS example

# Choose the correct path to AtlasWS
ATLASWS = $(abspath ../../AtlasWS)

vpath %.cpp src

# Compiler and flags
CXX = g++
CXXFLAGS = -fdiagnostics-color=always -std=c++20 -O3 
LFLAGS = -L$(ATLASWS)/build/ -lAtlasWSStatic

# Source files
SRC = src/main.cpp \
$(wildcard src/*.cpp) \

INC = -Isrc \
-I$(ATLASWS)/src \

# Output directory and library names
OUTDIR = build
EXEC = $(OUTDIR)/main

# Object files
OBJ = $(addprefix $(OUTDIR)/,$(notdir $(SRC:.cpp=.o)))

# Default target
all: $(EXEC)

# Rule to create the output directory
$(OUTDIR):
	mkdir -p $(OUTDIR)

# Rule to compile source files into object files
$(OUTDIR)/%.o: %.cpp | $(OUTDIR)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LFLAGS)

# Rule to clean the output directory
clean:
	rm -rf $(OUTDIR)