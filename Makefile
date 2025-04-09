# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Iinclude -Wall

# OS-specific setup
ifeq ($(OS),Windows_NT)
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = if not exist
    SEP = \\

    MAKE_DIR = \
        $(MKDIR) build mkdir build && \
        $(MKDIR) object mkdir object && \
        $(MKDIR) cache mkdir cache

    CLEAN_CMD = \
        if exist build $(RMDIR) build && \
        if exist object $(RMDIR) object && \
        if exist cache $(RMDIR) cache
else
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
    SEP = /

    MAKE_DIR = \
        $(MKDIR) build && \
        $(MKDIR) object && \
        $(MKDIR) cache

    CLEAN_CMD = $(RMDIR) build object cache
endif

# Linker flags
LDFLAGS = -lws2_32

# Source and object files
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:src/%.cpp=object/%.o)

# Output
TARGET = build$(SEP)server.exe

# Default
all: setup_dirs $(TARGET)
	@echo ✅ Build complete. Executable is at $(TARGET)
	@echo 🚀 To run the server: ./$(TARGET)
	@echo 🧹 To clean: make clean

setup_dirs:
	@$(MAKE_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

object/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(CLEAN_CMD)
