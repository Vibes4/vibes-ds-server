# Use an updated GCC image with build tools
FROM gcc:latest

# Set the working directory inside the container
WORKDIR /app

# Install required packages first
RUN apt-get update && apt-get install -y make g++

# Copy all source files, headers, and Makefile
COPY src/ src/
COPY include/ include/
COPY template/ template/
COPY Makefile Makefile

# Create the build directory
RUN mkdir -p build

# Copy the cache directory
RUN mkdir -p cache

# Add this before CMD
RUN touch /app/cache/key-value.db

# Compile using make
RUN make

# Ensure the binary is executable
RUN chmod +x build/server.exe

# Expose the server port (8080)
EXPOSE 8080

# Set volume mount point (optional but helpful for clarity)
VOLUME ["vibes-redis-cache"]

# Run the server when the container starts
CMD ["./build/server.exe"]
