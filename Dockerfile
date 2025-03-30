# Use a minimal base image with g++
FROM debian:latest

# Install dependencies
RUN apt update && apt install -y g++ make

# Set the working directory inside the container
WORKDIR /app

# Copy the source code and include headers
COPY src/ src/
COPY include/ include/

# Copy Makefile if you're using it
COPY Makefile .  # (Optional)

# Compile the server
RUN g++ -o build/server src/server.cpp src/main.cpp -Iinclude -lws2_32

# Expose the server port (8080)
EXPOSE 8080

# Run the server
CMD ["./build/server"]
