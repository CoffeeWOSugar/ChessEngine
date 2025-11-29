# Compiler and flags
CXX 		 := g++
CXXFLAGS := -std=c++17 -O3 -Wall -Wextra -pedantic

# Directories
SRC_DIR := src
BIN			:= chess

# Source files
SRCS := $(SRC_DIR)/main.cpp \
				$(SRC_DIR)/position.cpp \
				$(SRC_DIR)/movegen.cpp \
				$(SRC_DIR)/perft.cpp \
				$(SRC_DIR)/move.cpp

# Object files
OBJS := $(SRCS:.cpp=.o)

# Link
$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
.PHONY: clean
clean:
	rm -rf $(OBJS) $(BIN)
