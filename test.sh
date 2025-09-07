#!/bin/bash

echo "Testing P2P File Sharing System"
echo "==============================="

# Build if needed
if [ ! -f "build/tracker" ] || [ ! -f "build/peer" ]; then
    echo "Building project..."
    ./build.sh
fi

# Create test directory
mkdir -p test_output

# Start tracker in background
echo "Starting tracker..."
./build/tracker 8080 &
TRACKER_PID=$!
sleep 2

echo "Tracker started (PID: $TRACKER_PID)"

# Test basic connectivity
echo "Testing tracker connectivity..."
if ! nc -z 127.0.0.1 8080 2>/dev/null; then
    echo "Warning: Cannot connect to tracker (netcat not available or tracker not responding)"
else
    echo "Tracker is listening on port 8080"
fi

echo ""
echo "Manual testing required:"
echo "1. In terminal 2: ./build/peer 127.0.0.1 8080 9001"
echo "2. In peer 1: share test_files/sample.txt"
echo "3. In terminal 3: ./build/peer 127.0.0.1 8080 9002"
echo "4. In peer 2: download sample.txt test_output/downloaded.txt"
echo ""
echo "Press Enter to stop tracker..."
read

echo "Stopping tracker..."
kill $TRACKER_PID 2>/dev/null
wait $TRACKER_PID 2>/dev/null

echo "Test complete!"