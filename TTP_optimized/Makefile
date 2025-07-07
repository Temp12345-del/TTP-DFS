# Compiler and flags
CXX = mpicxx
CXXFLAGS = -fopenmp -O3

# Ensure the correct compiler is used by OpenMPI
OMPI_CXX = g++
export OMPI_CXX

# Source files
SRCS = generate_TTP.cpp parallel_TTP.cpp TTP_helpers.cpp

# Output executable
TARGET = run

# Build target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean target
clean:
	rm -f $(TARGET)
