#!/bin/bash

echo "🔬 Step-by-Step P2P Analysis"
echo "============================"

echo "📋 System Check:"
echo "Tracker status: $(lsof -i :8080 | grep LISTEN | wc -l | tr -d ' ') processes listening"
echo "Test files available:"
ls -la test_files/

echo ""
echo "🧪 Step 1: Share a file from Peer A"
echo "===================================="

# Create a named pipe for communication
mkfifo peer_a_input 2>/dev/null || true

# Start peer A in background
./build/peer 127.0.0.1 8080 9010 < peer_a_input &
PEER_A_PID=$!

# Give it time to start
sleep 2

# Share a file
echo "share test_files/sample.txt" > peer_a_input
sleep 2
echo "list" > peer_a_input
sleep 2

echo ""
echo "🧪 Step 2: Download from Peer B"
echo "==============================="

# Create another named pipe
mkfifo peer_b_input 2>/dev/null || true

# Start peer B
./build/peer 127.0.0.1 8080 9011 < peer_b_input &
PEER_B_PID=$!

sleep 2

# Try to download
echo "download test_files/sample.txt test_output/step_test.txt" > peer_b_input
sleep 5

echo ""
echo "📊 Results Analysis:"
echo "==================="

if [ -f "test_output/step_test.txt" ]; then
    echo "✅ SUCCESS: File downloaded!"
    echo "📏 Original size: $(wc -c < test_files/sample.txt) bytes"
    echo "📏 Downloaded size: $(wc -c < test_output/step_test.txt) bytes"
    
    if cmp -s test_files/sample.txt test_output/step_test.txt; then
        echo "✅ File integrity: PERFECT MATCH"
    else
        echo "⚠️  File integrity: MISMATCH"
    fi
else
    echo "❌ FAILED: No downloaded file found"
    echo "📁 Output directory:"
    ls -la test_output/ 2>/dev/null || echo "No output directory"
fi

# Cleanup
echo "quit" > peer_a_input 2>/dev/null
echo "quit" > peer_b_input 2>/dev/null
sleep 2

kill $PEER_A_PID $PEER_B_PID 2>/dev/null
wait $PEER_A_PID $PEER_B_PID 2>/dev/null

rm -f peer_a_input peer_b_input

echo ""
echo "🏁 Analysis Complete!"