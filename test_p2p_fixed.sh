#!/bin/bash

echo "🧪 Fixed P2P File Sharing Test"
echo "=============================="

# Create output directory
mkdir -p test_output

# Start peer A in background to share a file
echo "📤 Starting Peer A (Seeder) on port 9001..."
(
    echo "share test_files/sample.txt"
    echo "list"
    sleep 15  # Keep peer alive longer for download
    echo "quit"
) | ./build/peer 127.0.0.1 8080 9001 &
PEER_A_PID=$!

sleep 5  # Give more time for registration

# Start peer B to download the file (using full filename as registered)
echo "📥 Starting Peer B (Downloader) on port 9002..."
(
    sleep 2  # Wait a bit before downloading
    echo "download test_files/sample.txt test_output/downloaded_sample.txt"
    sleep 8  # Wait for download to complete
    echo "quit"
) | ./build/peer 127.0.0.1 8080 9002 &
PEER_B_PID=$!

# Wait for operations to complete
sleep 12

echo ""
echo "🔍 Test Results:"
echo "==============="

if [ -f "test_output/downloaded_sample.txt" ]; then
    echo "✅ File downloaded successfully!"
    echo "📊 Original file size: $(wc -c < test_files/sample.txt) bytes"
    echo "📊 Downloaded file size: $(wc -c < test_output/downloaded_sample.txt) bytes"
    
    if diff test_files/sample.txt test_output/downloaded_sample.txt > /dev/null; then
        echo "✅ File integrity verified - files are identical!"
        echo "📄 First few lines of downloaded file:"
        head -3 test_output/downloaded_sample.txt
    else
        echo "❌ File integrity check failed - files differ"
        echo "Showing differences:"
        diff test_files/sample.txt test_output/downloaded_sample.txt | head -10
    fi
else
    echo "❌ Download failed - file not found"
    echo "📁 Contents of test_output directory:"
    ls -la test_output/
fi

# Cleanup
kill $PEER_A_PID $PEER_B_PID 2>/dev/null
wait $PEER_A_PID $PEER_B_PID 2>/dev/null

echo ""
echo "🏁 Test completed!"