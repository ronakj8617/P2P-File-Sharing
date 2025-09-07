#!/bin/bash

echo "🧪 Automated P2P File Sharing Test"
echo "=================================="

# Create output directory
mkdir -p test_output

# Start peer A in background to share a file
echo "📤 Starting Peer A (Seeder) on port 9001..."
(
    echo "share test_files/sample.txt"
    echo "list"
    sleep 10  # Keep peer alive for download
    echo "quit"
) | ./build/peer 127.0.0.1 8080 9001 &
PEER_A_PID=$!

sleep 3

# Start peer B to download the file
echo "📥 Starting Peer B (Downloader) on port 9002..."
(
    echo "download sample.txt test_output/downloaded_sample.txt"
    sleep 5
    echo "quit"
) | ./build/peer 127.0.0.1 8080 9002 &
PEER_B_PID=$!

# Wait for operations to complete
sleep 8

echo ""
echo "🔍 Test Results:"
echo "==============="

if [ -f "test_output/downloaded_sample.txt" ]; then
    echo "✅ File downloaded successfully!"
    echo "📊 Original file size: $(wc -c < test_files/sample.txt) bytes"
    echo "📊 Downloaded file size: $(wc -c < test_output/downloaded_sample.txt) bytes"
    
    if diff test_files/sample.txt test_output/downloaded_sample.txt > /dev/null; then
        echo "✅ File integrity verified - files are identical!"
    else
        echo "❌ File integrity check failed - files differ"
    fi
else
    echo "❌ Download failed - file not found"
fi

# Cleanup
kill $PEER_A_PID $PEER_B_PID 2>/dev/null
wait $PEER_A_PID $PEER_B_PID 2>/dev/null

echo ""
echo "🏁 Test completed!"