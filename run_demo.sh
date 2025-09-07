#!/bin/bash

echo "🚀 P2P File Sharing System Demo"
echo "================================"

# Check if tracker is running
if ! lsof -i :8080 > /dev/null 2>&1; then
    echo "Starting tracker server..."
    ./build/tracker 8080 &
    TRACKER_PID=$!
    sleep 2
    echo "Tracker started (PID: $TRACKER_PID)"
else
    echo "✅ Tracker is already running on port 8080"
fi

echo ""
echo "📁 Available test files:"
ls -la test_files/

echo ""
echo "📊 File sizes:"
wc -c test_files/*

echo ""
echo "🔧 Testing basic peer functionality..."

# Test peer startup
echo "Starting peer on port 9001..."
timeout 5s ./build/peer 127.0.0.1 8080 9001 <<EOF &
help
quit
EOF

sleep 2

echo ""
echo "✅ Demo setup complete!"
echo ""
echo "🎯 Manual Testing Instructions:"
echo "==============================="
echo ""
echo "Terminal 1 - Peer A (File Sharer):"
echo "  ./build/peer 127.0.0.1 8080 9001"
echo "  > share test_files/sample.txt"
echo "  > list"
echo ""
echo "Terminal 2 - Peer B (File Downloader):"
echo "  ./build/peer 127.0.0.1 8080 9002"
echo "  > download sample.txt downloaded_sample.txt"
echo ""
echo "Terminal 3 - Verify Download:"
echo "  diff test_files/sample.txt downloaded_sample.txt"
echo ""
echo "🛑 To stop tracker: pkill tracker"