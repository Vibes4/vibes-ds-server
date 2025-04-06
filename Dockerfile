# Use an updated GCC image with build tools
FROM gcc:latest

# Set the working directory inside the container
WORKDIR /usr/src/redis-server

# Copy all source files, headers, and Makefile
COPY . .

# Create the build directory
RUN mkdir -p build

# Compile the server
RUN g++ -std=c++11 -o build/server src/main.cpp src/server.cpp src/helper.cpp src/kv_store.cpp -Iinclude -lpthread

# Ensure the binary is executable
RUN chmod +x build/server

# Expose the server port (8080)
EXPOSE 8080

# Run the server when the container starts
CMD ["./build/server"]
