#!/bin/bash

echo "🐛 Debug P2P File Transfer"
echo "=========================="

# Kill any existing peers
pkill peer 2>/dev/null

# Create a simple test file
echo "Hello P2P World! This is a test file for debugging." > test_output/debug_test.txt

echo "📁 Test file created:"
ls -la test_output/debug_test.txt
echo "Content: $(cat test_output/debug_test.txt)"

echo ""
echo "🚀 Starting peer A to share the debug file..."

# Start peer A in background with verbose output
./build/peer 127.0.0.1 8080 9005 &
PEER_A_PID=$!

sleep 2

# Send commands to peer A
echo "share test_output/debug_test.txt" | nc localhost 9005 2>/dev/null || echo "Could not connect to peer A"

sleep 2

echo ""
echo "🔍 Starting peer B to download..."

# Start peer B
./build/peer 127.0.0.1 8080 9006 &
PEER_B_PID=$!

sleep 2

# Try to download
echo "download debug_test.txt test_output/downloaded_debug.txt" | nc localhost 9006 2>/dev/null || echo "Could not connect to peer B"

sleep 3

echo ""
echo "📊 Results:"
if [ -f "test_output/downloaded_debug.txt" ]; then
    echo "✅ Debug file downloaded!"
    echo "Original: $(cat test_output/debug_test.txt)"
    echo "Downloaded: $(cat test_output/downloaded_debug.txt)"
else
    echo "❌ Debug download failed"
fi

# Cleanup
kill $PEER_A_PID $PEER_B_PID 2>/dev/null
wait $PEER_A_PID $PEER_B_PID 2>/dev/null

echo "🏁 Debug test completed"