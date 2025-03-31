# Use a minimal base image with g++
FROM debian:latest

# Install dependencies
RUN apt update && apt install -y g++ make

# Set the working directory inside the container
WORKDIR /app

# Copy the source code and include headers
COPY src/ src/
COPY include/ include/
COPY template/ template/

# Copy Makefile if you're using it
COPY Makefile .

# Compile the server
RUN mingw32-make

# Expose the server port (8080)
EXPOSE 8080

# Run the server
CMD ["./build/server"]
