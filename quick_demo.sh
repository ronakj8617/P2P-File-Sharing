#!/bin/bash

echo "🚀 P2P File Sharing System Demo"
echo "================================"

# Kill any existing processes
pkill -f "./build/tracker" 2>/dev/null
pkill -f "./build/peer" 2>/dev/null
sleep 1

echo "📡 Starting tracker server on port 8080..."
./build/tracker 8080 &
TRACKER_PID=$!
sleep 2

echo "👤 Starting Peer 1 (Seeder) on port 9001..."
echo "📁 Sharing test_files/sample.txt..."

# Create a script for peer 1 commands
cat > peer1_commands.txt << EOF
share test_files/sample.txt
list
EOF

# Start peer 1 and share file
timeout 10s ./build/peer 127.0.0.1 8080 9001 < peer1_commands.txt &
PEER1_PID=$!
sleep 3

echo "👤 Starting Peer 2 (Downloader) on port 9002..."
echo "⬇️  Downloading sample.txt..."

# Create a script for peer 2 commands  
cat > peer2_commands.txt << EOF
download sample.txt downloaded_sample.txt
quit
EOF

# Start peer 2 and download file
timeout 10s ./build/peer 127.0.0.1 8080 9002 < peer2_commands.txt
sleep 2

echo ""
echo "✅ Demo Results:"
echo "================"

if [ -f "downloaded_sample.txt" ]; then
    echo "✓ File downloaded successfully!"
    echo "📊 Original size: $(wc -c < test_files/sample.txt) bytes"
    echo "📊 Downloaded size: $(wc -c < downloaded_sample.txt) bytes"
    
    if diff test_files/sample.txt downloaded_sample.txt > /dev/null; then
        echo "✓ Files are identical - P2P transfer successful! 🎉"
    else
        echo "❌ Files differ - transfer incomplete"
    fi
else
    echo "❌ Download failed"
fi

# Cleanup
echo ""
echo "🧹 Cleaning up..."
kill $TRACKER_PID $PEER1_PID 2>/dev/null
rm -f peer1_commands.txt peer2_commands.txt
wait 2>/dev/null

echo "Demo complete!"