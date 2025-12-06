# Compiler and flags
CXX 		 := g++
CXXFLAGS := -std=c++17 -O3 -Wall -Wextra -pedantic

# Directories
SRC_DIR := src
TST_DIR := tests
BIN			:= chess

# Source files
SRCS := $(SRC_DIR)/main.cpp \
				$(SRC_DIR)/position.cpp \
				$(SRC_DIR)/movegen.cpp \
				$(SRC_DIR)/perft.cpp \
				$(SRC_DIR)/move.cpp \
				$(SRC_DIR)/search.cpp \
				$(TST_DIR)/perft_tests.cpp

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

# Format
.PHONY: format
format:
	clang-format -i $(SRC_DIR)/*.cpp $(SRC_DIR)/*.h $(TST_DIR)/*.cpp $(TST_DIR)/*.h

