#!/bin/bash

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build complete!"
echo "Executables:"
echo "  ./tracker [port] - Start tracker server (default port 8080)"
echo "  ./peer <tracker_ip> <tracker_port> <peer_port> - Start peer node"