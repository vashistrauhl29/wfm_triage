FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    libasio-dev \
    libssl-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install Crow (header-only) so find_package(Crow) resolves at /usr/local/lib/cmake/Crow
RUN git clone --depth 1 https://github.com/CrowCpp/Crow.git /tmp/crow \
    && cmake -S /tmp/crow -B /tmp/crow/build \
    && cmake --install /tmp/crow/build \
    && rm -rf /tmp/crow

WORKDIR /app
COPY . .

RUN cmake . -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && make uber_wfm_backend -j$(nproc)

EXPOSE 8080

CMD ["/app/uber_wfm_backend"]
