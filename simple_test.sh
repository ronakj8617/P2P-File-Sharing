#!/bin/bash

echo "🔧 Simple P2P Test"
echo "=================="

echo "📋 System Status:"
echo "Tracker running: $(lsof -i :8080 | grep LISTEN | wc -l) processes"
echo "Available files:"
ls -la test_files/

echo ""
echo "🧪 Testing peer startup and file sharing..."

# Test 1: Start a peer and share a file
echo "Test 1: Starting peer and sharing file..."
echo -e "share test_files/sample.txt\nlist\nquit" | ./build/peer 127.0.0.1 8080 9003

echo ""
echo "Test 2: Query tracker for available files..."
# We can't directly query the tracker, but we can start another peer and try to download

echo "Starting second peer to test download..."
echo -e "download test_files/sample.txt test_output/test_download.txt\nquit" | ./build/peer 127.0.0.1 8080 9004

echo ""
echo "📊 Results:"
if [ -f "test_output/test_download.txt" ]; then
    echo "✅ Download successful!"
    echo "File size: $(wc -c < test_output/test_download.txt) bytes"
else
    echo "❌ Download failed"
    echo "Output directory contents:"
    ls -la test_output/ 2>/dev/null || echo "No output directory"
fi