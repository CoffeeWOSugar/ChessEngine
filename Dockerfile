# ================== Build stage ==================
FROM debian:stable-slim AS build

# Install build tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy project files into the image
COPY CMakeLists.txt .
COPY src src
COPY tests tests

# Configure + build
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --config Release -j

# ================== Runtime stage ==================
FROM debian:stable-slim

# Install runtime dependencies only
RUN apt-get update && apt-get install -y --no-install-recommends \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the built binary from the builder stage
COPY --from=build /app/build/chess /usr/local/bin/chess

# set TERM to something reasonable for TUI
ENV TERM xterm-256color

# Default command
CMD ["chess"]

