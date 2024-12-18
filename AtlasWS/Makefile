# 	Makefile for building AtlasWS Library
# 
# 	Output:
# 		- build/libAtlasWS.so (shared library)
# 		- build/libAtlasWSStatic.a (static library)


LIBNAMEBASE = libAtlasWS

vpath %.cpp src \
src \
src/util \
src/http-transfer \
src/http-range \
src/socket \
src/time \
src/status-code \
src/error \
src/handler \
src/http-parse \
src/file \
src/websocket \

# Compiler and flags
CXX = g++

#CXXFLAGS = -fdiagnostics-color=always -std=c++20 -O2 -g # debug
CXXFLAGS = -fdiagnostics-color=always -Wall -Wextra -Wpedantic -Wno-unused-command-line-argument -std=c++20 -fPIC -O3 # release


# Source files
SRC = $(wildcard src/*.cpp) \
$(wildcard src/*.cpp) \
$(wildcard src/util/*.cpp) \
$(wildcard src/http-transfer/*.cpp) \
$(wildcard src/http-range/*.cpp) \
$(wildcard src/socket/*.cpp) \
$(wildcard src/time/*.cpp) \
$(wildcard src/status-code/*.cpp) \
$(wildcard src/error/*.cpp) \
$(wildcard src/handler/*.cpp) \
$(wildcard src/http-parse/*.cpp) \
$(wildcard src/file/*.cpp) \
$(wildcard src/websocket/*.cpp) \

INC = src \
src/util \
src/http-transfer \
src/http-range \
src/socket \
src/time \
src/status-code \
src/error \
src/handler \
src/http-parse \
src/file \
src/websocket \


# Output directory and library names
OUTDIR = build
LIB = $(OUTDIR)/$(LIBNAMEBASE).so
STATICLIB = $(OUTDIR)/$(LIBNAMEBASE)Static.a

# Object files
OBJ = $(addprefix $(OUTDIR)/,$(notdir $(SRC:.cpp=.o)))


# Default target
all: $(LIB) $(STATICLIB)

# Rule to create the output directory
$(OUTDIR):
	mkdir -p $(OUTDIR)

# Rule to compile source files into object files
$(OUTDIR)/%.o: %.cpp | $(OUTDIR)
	$(CXX) $(CXXFLAGS) -I$(INC) -c $< -o $@

# Rule to link object files into shared library
$(LIB): $(OBJ)
	$(CXX) $(CXXFLAGS) -shared $^ -o $@

# Rule to create static library
$(STATICLIB): $(OBJ)
	ar rcs $@ $^

# Rule to clean the output directory
clean:
	rm -rf $(OUTDIR)