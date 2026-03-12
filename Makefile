# Compiler and flags
CXX 		 := g++
CXXFLAGS := -std=c++20 -Iinclude -Wall -Wextra -pedantic

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
				$(SRC_DIR)/engine_session.cpp \
				$(SRC_DIR)/opening_book.cpp \
				$(TST_DIR)/perft_tests.cpp

# Object files
OBJS := $(SRCS:.cpp=.o)

# Link
$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Format
.PHONY: format
format:
	clang-format -i $(SRC_DIR)/*.cpp $(SRC_DIR)/*.h $(TST_DIR)/*.cpp $(TST_DIR)/*.h

MPI_CXX      := mpicxx
MPI_CXXFLAGS := $(CXXFLAGS) -DUSE_MPI
MPI_BIN      := chess_mpi

MPI_OBJS := $(SRCS:.cpp=.mpi.o)

# Link MPI binary
$(MPI_BIN): $(MPI_OBJS)
	$(MPI_CXX) $(MPI_CXXFLAGS) -o $@ $^

# Compile MPI objects
%.mpi.o: %.cpp
	$(MPI_CXX) $(MPI_CXXFLAGS) -c $< -o $@

.PHONY: mpi
mpi: $(MPI_BIN)

# Clean up
.PHONY: clean
clean:
	rm -rf $(OBJS) $(MPI_OBJS) $(BIN) $(MPI_BIN)

