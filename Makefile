# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Iinclude -Wall

# Linker flags (commented out Windows-specific flag for Linux build)
# LDFLAGS = -lws2_32
LDFLAGS = -lpthread

# Source files
SRCS = $(wildcard src/*.cpp)

# Object files (store .o files in object/)
OBJS = $(SRCS:src/%.cpp=object/%.o)

# Output executable
TARGET = build/server.exe

# Default rule
all: setup_dirs $(TARGET)
	@echo "✅ Build complete. Executable is located at $(TARGET)"
	@echo "🚀 To run the server: ./$(TARGET)"
	@echo "🧹 To clean up: make clean"

# Create necessary directories
setup_dirs:
	@mkdir -p build object cache

# Linking the final executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compiling each .cpp file into a .o file
object/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove compiled files
clean:
	rm -rf object build
