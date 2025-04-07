# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Iinclude -Wall

# Linker flags
LDFLAGS = -lws2_32

# Source files
SRCS = $(wildcard src/*.cpp) 

# Object files (store .o files in object/)
OBJS = $(SRCS:src/%.cpp=object/%.o)

# Output executable
TARGET = build/server.exe

# Default rule
all: setup_dirs $(TARGET)

# Create necessary directories
setup_dirs:
	@if not exist build mkdir build
	@if not exist object mkdir object

# Linking the final executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compiling each .cpp file into a .o file
object/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove compiled files
clean:
	del /Q object\*.o build\server.exe 2>nul || exit 0
