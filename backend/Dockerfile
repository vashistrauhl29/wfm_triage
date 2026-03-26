# ── Stage 1: Build ────────────────────────────────────────────────────────────
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    git \
    libboost-all-dev \
    libssl-dev \
    wget \
    zip \
    unzip \
    && rm -rf /var/lib/apt/lists/*

# Install Crow header-only framework so find_package(Crow) resolves
RUN git clone --depth 1 https://github.com/CrowCpp/Crow.git /tmp/crow \
    && cmake -S /tmp/crow -B /tmp/crow/build \
    && cmake --install /tmp/crow/build \
    && rm -rf /tmp/crow

WORKDIR /app
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target uber_wfm_backend -j$(nproc)

# ── Stage 2: Runtime ──────────────────────────────────────────────────────────
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libboost-system-dev \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/uber_wfm_backend /usr/local/bin/uber_wfm_backend

EXPOSE 8080

CMD ["uber_wfm_backend"]
