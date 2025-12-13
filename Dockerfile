# ================== Build stage ==================
FROM debian:stable-slim AS build

# Install build tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy project files into the image
COPY CMakeLists.txt .
COPY include include
COPY src src
COPY tests tests

# Configure + build
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --config Release -j

# ================== Runtime stage ==================
FROM debian:stable-slim

WORKDIR /app

# Install runtime dependencies only
RUN apt-get update && apt-get install -y --no-install-recommends \
  python3 python3-venv \
 && rm -rf /var/lib/apt/lists/*

# Copy the built binary from the builder stage
COPY --from=build /app/build/chess /usr/local/bin/chess

# Copy web app + Python dependencies
COPY web web
COPY requirements.txt .

# Create virtual environment
RUN python3 -m venv /venv \
  && /venv/bin/pip install --no-cache-dir -r requirements.txt

# set TERM to something reasonable for TUI
ENV PATH="/venv/bin:$PATH"
ENV TERM xterm-256color

EXPOSE 8000

# Run FastAPI via uvicorn
CMD ["uvicorn", "web.app:app", "--host", "0.0.0.0", "--port", "8000"]

# Default command
# CMD ["chess"]

